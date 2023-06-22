/******************************************************************************
  Copyright (c) 1992, 1995, 1996 Xerox Corporation.  All rights reserved.
  Portions of this code were written by Stephen White, aka ghond.
  Use and copying of this software and preparation of derivative works based
  upon this software are permitted.  Any distribution of this software or
  derivative works must comply with all applicable United States export
  control laws.  This software is made available AS IS, and Xerox Corporation
  makes no warranty about the software, its performance or its conformity to
  any specification.  Any person obtaining a copy of this software is requested
  to send their name and post office or electronic mail address to:
    Pavel Curtis
    Xerox PARC
    3333 Coyote Hill Rd.
    Palo Alto, CA 94304
    Pavel@Xerox.Com
 *****************************************************************************/

#include <stdlib.h>
#include <string>
#include <vector>

#include "db.h"
#include "list.h"
#include "log.h"
#include "match.h"
#include "server.h"
#include "utils.h"

#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>
#include <rapidfuzz/fuzz.hpp>

inline std::vector<std::string> 
get_name(Objid oid) 
{
    std::vector<std::string> tokens;
    std::string name = db_object_name(oid);
    return ts::string::explode(ts::string::tolowercase(name));
}

inline std::vector<std::string> 
get_aliases(Objid oid) 
{
    std::vector<std::string> tokens;

    Var db_aliases;
    db_prop_handle h = db_find_property(Var::new_obj(oid), "aliases", &db_aliases);
    if (h.ptr && (db_aliases.type == TYPE_LIST || db_aliases.type == TYPE_STR))
        tokens = ts::string::explode(ts::string::tolowercase(ts::string::join(ts::var::var_to_vector(db_aliases))));

    return tokens; 
}

std::vector<Objid> 
get_contents(Objid oid) 
{
    std::vector<Objid> stuff;

    if (!valid(oid)) return stuff;

    Var contents;
    db_prop_handle h = db_find_property(Var::new_obj(oid), "contents", &contents);
    if(h.ptr && contents.v.list[0].v.num > 0)
        for(auto c=1; c<=contents.v.list[0].v.num; c++)
            stuff.emplace_back(contents.v.list[c].v.obj);

    return stuff;
}

std::vector<Objid> 
nearby_objects(Objid where) 
{
    int step;
    Objid oid, loc;
    loc = db_object_location(where);

    std::vector<Objid> contents;

    if(valid(loc))
        contents = ts::utils::merge_unique<Objid>(get_contents(where), get_contents(loc));
    else
        contents = get_contents(where);

    return contents;
}

inline 
std::string name_and_aliases(Objid oid)
{
    std::vector<std::string> name_and_aliases = ts::utils::merge_unique<std::string>(get_name(oid), get_aliases(oid));
    return ts::string::join(name_and_aliases);
}

Objid 
match_object(Objid player, const char *name, Var *object_list)
{
    std::string search_name = name;

    if (search_name.empty())
        return NOTHING;
    if (search_name[0] == '#' && is_wizard(player)) {
        Objid r;
        r = static_cast<Objid>(std::stoi(search_name.substr(1, search_name.size()-1)));
        if(!valid(r)) return FAILED_MATCH;
        return r;
    }

    if (!valid(player))
        return FAILED_MATCH;
    if (search_name == "me" || search_name == "myself")
        return player;
    if (search_name == "here")
        return db_object_location(player);

    std::vector<std::string> targets;
    std::vector<Objid> contents = nearby_objects(player);

    targets.reserve(contents.size());

    for(auto& o: contents) {
        if(!valid(o)) continue;
        targets.emplace_back(name_and_aliases(o));
    }

    targets.shrink_to_fit();

    std::vector<int> matches = complex_match(search_name, targets, server_int_option("match_threshold", 70));

    int n_matches = matches.size();

    if (n_matches == 1) 
        return contents[matches.front()];
    else if (n_matches <= 0) 
        return FAILED_MATCH;
    else
        return AMBIGUOUS;
}

std::pair<int, std::string> 
parse_ordinal(std::string word) 
{
    if (auto search = english_ordinals.find(word); search != english_ordinals.end()) {
        return std::make_pair(search->second, "");
    }

    std::pair<int, std::string> result = std::make_pair(FAILED_MATCH, word);
    
    std::smatch sm;

    if (std::regex_search(word, sm, match_numeric) && sm.size() > 1) {
        try { // Matching for 1.thing, 2.thing, etc.
            result = std::make_pair(std::stoi(sm.str(1)), word.substr(sm.str(0).size()));
        } catch (...) { }
    }

    if (std::regex_search(word, sm, match_ordinal) && sm.size() > 1) {
        try { // Third order matching, try matching 1st, etc.
            result = std::make_pair(std::stoi(sm.str(1)), "");
        } catch (...) { }
    }

    return result;
}

bool 
token_match(Objid *match, Objid speaker, const char *token, std::vector<Objid> oids) 
{
    auto n = oids.size();

    if(n == 0) return false;

    Var objects = ts::var::vec_to_list<Objid>(oids);

    Objid res;
    res = match_object(speaker, token, &objects);

    if(valid(res)) {
      *match = res;
      return true;
    }

    return false;
}

static inline double 
fuzzy_match_score(std::string subject, std::string target)
{
    double token_ratio, token_sort, token_set, score;

    token_ratio = rapidfuzz::fuzz::ratio(subject, target);
    token_sort  = rapidfuzz::fuzz::partial_token_sort_ratio(subject, target);
    token_set   = rapidfuzz::fuzz::token_set_ratio(subject, target);

    score = std::ceil(RMS_BEST2(token_ratio, token_set, token_sort));

    #if MATCH_DEBUG > 0
    oklog("%s <=> %s [%f %f %f] %f\n", subject.c_str(), target.c_str(), token_ratio, token_set, token_sort, score);
    #endif 

    return score;
}

std::vector<int> 
fuzzy_match(std::string subject, std::vector<std::string> targets, double threshold, bool use_ordinal)
{
    auto sz = targets.size();
    std::vector<int> matches;
    
    if (sz <= 0) return matches;

    matches.reserve(sz);

    int ordinal = 0;

    std::string match_subject;    
    if(!use_ordinal) {
        match_subject = subject;
    } else {
        std::vector<std::string> tokens = ts::string::explode(subject);
        std::tie(ordinal, tokens[0])    = parse_ordinal(tokens[0]);

        if(tokens[0] == "") tokens.erase(tokens.begin());
        match_subject = ts::string::join(tokens);
    }

    for(int i = 0; i < targets.size(); i++)
        if(fuzzy_match_score(match_subject, targets[i]) >= threshold) matches.emplace_back(i);

    if(ordinal > 0) {
        if(ordinal > matches.size()) return fuzzy_match(subject, targets, threshold, false);
        int match = matches[ordinal - 1];
        matches.clear();
        matches.emplace_back(match);
    }

    matches.shrink_to_fit();

    return matches;
}

std::vector<int> 
complex_match(std::string subject, std::vector<std::string> targets, int threshold)
{
    std::vector<int> matches = fuzzy_match(subject, targets, CLAMP100(threshold), true);
    return matches;
}
