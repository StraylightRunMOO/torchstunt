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

#ifndef EXT_LIST_H
#define EXT_LIST_H 1

#include <stdlib.h>
#include <algorithm> // std::sort
#include <map>
#include <regex>
#include <string>
#include <vector>

#include "structures.h"
#include "streams.h"

#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/as_array.hpp>

extern Var new_list(int size);
extern void destroy_list(Var list);
extern Var list_dup(Var list);

extern Var listappend(Var list, Var value);
extern Var listinsert(Var list, Var value, int pos);
extern Var listdelete(Var list, int pos);
extern Var listset(Var list, Var value, int pos);
extern Var listrangeset(Var list, int from, int to, Var value);
extern Var listconcat(Var first, Var second);
extern Var setadd(Var list, Var value);
extern Var setremove(Var list, Var value);
extern Var sublist(Var list, int lower, int upper);
extern int listequal(Var lhs, Var rhs, int case_matters);

extern int list_sizeof(Var *list);

typedef int (*listfunc) (Var value, void *data, int first);
extern int listforeach(Var list, listfunc func, void *data);

extern Var strrangeset(Var list, int from, int to, Var value);
extern Var substr(Var str, int lower, int upper);
extern Var strget(Var str, int i);

extern const char *value2str(Var);
extern void unparse_value(Stream *, Var);

/*
 * Returns the length of the given list `l'.  Does *not* check to
 * ensure `l' is, in fact, a list.
 */
static inline Num
listlength(Var l)
{
    return l.v.list[0].v.num;
}

/*
 * Wraps `v' in a list if it is not already a list.  Consumes `v', so
 * you may want to var_ref/var_dup `v'.  Currently, this function is
 * called by functions that operate on an object's parents, which can
 * be either an object reference (TYPE_OBJ) or a list of object
 * references (TYPE_LIST).
 */
static inline Var
enlist_var(Var v)
{
    if (TYPE_LIST == v.type)
	return v;

    Var r = new_list(1);
    r.v.list[1] = v;
    return r;
}

/*
 * Iterate over the values in the list `lst'.  Sets `val' to each
 * value, in turn.  `itr' and `cnt' must be int variables.  In the
 * body of the statement, they hold the current index and total count,
 * respectively.  Use the macro as follows (assuming you already have
 * a list in `items'):
 *   Var item;
 *   int i, c;
 *   FOR_EACH(item, items, i, c) {
 *       printf("%d of %d, item = %s\n", i, c, value_to_literal(item));
 *   }
 */
#define FOR_EACH(val, lst, idx, cnt)				\
for (idx = 1, cnt = lst.v.list[0].v.num;			\
     idx <= cnt && (val = lst.v.list[idx], 1);			\
     idx++)

/*
 * Pop the first value off `stck' and put it in `tp'.
 */
#define POP_TOP(tp, stck)					\
tp = var_ref(stck.v.list[1]);					\
stck = listdelete(stck, 1);
#endif

namespace ts {
  namespace string {

    static inline std::vector<std::string> 
    explode(std::string_view str, std::string_view sep = " ") {
        std::vector<std::string> words;
        boost::split(words, str, boost::is_any_of(sep));
        return words;
    }

    static inline std::string 
    join(std::vector<std::string> words, std::string sep = " ") {
        return boost::algorithm::join(words, sep);
    }

    static inline std::string 
    tolowercase(std::string str) {
        return boost::algorithm::to_lower_copy(str);
    }

    static inline std::string
    trim(std::string str, std::string_view delim = " ") {
      boost::trim_if(str, boost::is_any_of(boost::as_array(delim)));
      return str;
    }

    static inline Var vec_to_list(std::vector<std::string> v, char mode) {
      auto n  = v.size();
      Var ret = new_list(n);

      if(n==0) return ret;

      auto i = 0;
      for(auto& x: v)
        if(!x.empty() || mode == 1)
          ret.v.list[++i] = str_dup_to_var(x.data());

      ret.v.list[0].v.num = i;

      return ret;
    }
  }
}

static const std::vector<std::regex> UNCOUNTABLE_RULES = {
    std::regex("adulthood"), std::regex("advice"), std::regex("agenda"),
    std::regex("aid"), std::regex("aircraft"), std::regex("alcohol"),
    std::regex("ammo"), std::regex("analytics"), std::regex("anime"),
    std::regex("athletics"), std::regex("audio"), std::regex("bison"),
    std::regex("blood"), std::regex("bream"), std::regex("buffalo"),
    std::regex("butter"), std::regex("carp"), std::regex("cash"),
    std::regex("chassis"), std::regex("chess"), std::regex("clothing"),
    std::regex("cod"), std::regex("commerce"), std::regex("cooperation"),
    std::regex("corps"), std::regex("debris"), std::regex("diabetes"),
    std::regex("digestion"), std::regex("elk"), std::regex("energy"),
    std::regex("equipment"), std::regex("excretion"), std::regex("expertise"),
    std::regex("firmware"), std::regex("flounder"), std::regex("fun"),
    std::regex("gallows"), std::regex("garbage"), std::regex("graffiti"),
    std::regex("hardware"), std::regex("headquarters"), std::regex("health"),
    std::regex("herpes"), std::regex("highjinks"), std::regex("homework"),
    std::regex("housework"), std::regex("information"), std::regex("jeans"),
    std::regex("justice"), std::regex("kudos"), std::regex("labour"),
    std::regex("literature"), std::regex("machinery"), std::regex("mackerel"),
    std::regex("mail"), std::regex("media"), std::regex("mews"),
    std::regex("moose"), std::regex("music"), std::regex("mud"),
    std::regex("manga"), std::regex("news"), std::regex("only"),
    std::regex("personnel"), std::regex("pike"), std::regex("plankton"),
    std::regex("pliers"), std::regex("police"), std::regex("pollution"),
    std::regex("premises"), std::regex("rain"), std::regex("research"),
    std::regex("rice"), std::regex("salmon"), std::regex("scissors"),
    std::regex("series"), std::regex("sewage"), std::regex("shambles"),
    std::regex("shrimp"), std::regex("software"), std::regex("staff"),
    std::regex("swine"), std::regex("tennis"), std::regex("traffic"),
    std::regex("transportation"), std::regex("trout"), std::regex("tuna"),
    std::regex("wealth"), std::regex("welfare"), std::regex("whiting"),
    std::regex("wildebeest"), std::regex("wildlife"), std::regex("you"),
    std::regex("pok[e]mon$"), std::regex("[^aeiou]ese$"), std::regex("deer$"),
    std::regex("fish$"), std::regex("measles$"), std::regex("o[iu]s$")
};

static const std::map<std::string, std::string> IRREGULAR_SINGULAR_RULES = {
    { "anathema", "anathemata" },
    { "axe", "axes" },
    { "canvas", "canvases" },
    { "carve", "carves" },
    { "die", "dice" },
    { "dingo", "dingoes" },
    { "dogma", "dogmata" },
    { "eave", "eaves" },
    { "echo", "echoes" },
    { "foot", "feet" },
    { "genus", "genera" },
    { "go", "goes" },
    { "goose", "geese" },
    { "groove", "grooves" },
    { "has", "have" },
    { "he", "they" },
    { "her", "their" },
    { "herself", "themselves" },
    { "himself", "themselves" },
    { "his", "their" },
    { "human", "humans" },
    { "I", "we" },
    { "is", "are" },
    { "its", "their" },
    { "itself", "themselves" },
    { "lemma", "lemmata" },
    { "looey", "looies" },
    { "me", "us" },
    { "my", "our" },
    { "myself", "ourselves" },
    { "ox", "oxen" },
    { "passerby", "passersby" },
    { "pickaxe", "pickaxes" },
    { "proof", "proofs" },
    { "quiz", "quizzes" },
    { "schema", "schemata" },
    { "she", "they" },
    { "stigma", "stigmata" },
    { "stoma", "stomata" },
    { "that", "those" },
    { "them", "them" },
    { "themself", "themselves" },
    { "thief", "thieves" },
    { "this", "these" },
    { "tooth", "teeth" },
    { "tornado", "tornadoes" },
    { "torpedo", "torpedoes" },
    { "valve", "valves" },
    { "viscus", "viscera" },
    { "volcano", "volcanoes" },
    { "was", "were" },
    { "yes", "yeses" },
    { "yourself", "yourselves" },
};

static const std::map<std::string, std::string> IRREGULAR_PLURAL_RULES = {
    { "anathemata", "anathema" },
    { "are", "is" },
    { "axes", "axe" },
    { "canvases", "canvas" },
    { "carves", "carve" },
    { "dice", "die" },
    { "dingoes", "dingo" },
    { "dogmata", "dogma" },
    { "eaves", "eave" },
    { "echoes", "echo" },
    { "feet", "foot" },
    { "geese", "goose" },
    { "genera", "genus" },
    { "goes", "go" },
    { "grooves", "groove" },
    { "have", "has" },
    { "humans", "human" },
    { "lemmata", "lemma" },
    { "looies", "looey" },
    { "our", "my" },
    { "ourselves", "myself" },
    { "oxen", "ox" },
    { "passersby", "passerby" },
    { "pickaxes", "pickaxe" },
    { "proofs", "proof" },
    { "quizzes", "quiz" },
    { "schemata", "schema" },
    { "stigmata", "stigma" },
    { "stomata", "stoma" },
    { "teeth", "tooth" },
    { "their", "its" },
    { "them", "them" },
    { "themselves", "themself" },
    { "these", "this" },
    { "they", "she" },
    { "thieves", "thief" },
    { "those", "that" },
    { "tornadoes", "tornado" },
    { "torpedoes", "torpedo" },
    { "us", "me" },
    { "valves", "valve" },
    { "viscera", "viscus" },
    { "volcanoes", "volcano" },
    { "we", "I" },
    { "were", "was" },
    { "yeses", "yes" },
    { "yourselves", "yourself" }
};

static const std::vector<std::tuple<std::regex, std::string>> SINGULARIZATION_RULES = {
    std::tuple<std::regex, std::string>( std::regex("(agend|addend|millenni|dat|extrem|bacteri|desiderat|strat|candelabr|errat|ov|symposi|curricul|quor)a$"), "$1um" ),
    std::tuple<std::regex, std::string>( std::regex("(alumn|alg|vertebr)ae$"), "$1a" ),
    std::tuple<std::regex, std::string>( std::regex("(alumn|syllab|vir|radi|nucle|fung|cact|stimul|termin|bacill|foc|uter|loc|strat)(?:us|i)$"), "$1us" ),
    std::tuple<std::regex, std::string>( std::regex("(analy|diagno|parenthe|progno|synop|the|empha|cri|ne)(?:sis|ses)$"), "$1sis" ),
    std::tuple<std::regex, std::string>( std::regex("(apheli|hyperbat|periheli|asyndet|noumen|phenomen|criteri|organ|prolegomen|hedr|automat)a$"), "$1on" ),
    std::tuple<std::regex, std::string>( std::regex("(ar|(?:wo|[ae])l|[eo][ao])ves$"), "$1f" ),
    std::tuple<std::regex, std::string>( std::regex("(child)ren$"), "$1" ),
    std::tuple<std::regex, std::string>( std::regex("(cod|mur|sil|vert|ind)ices$"), "$1ex" ),
    std::tuple<std::regex, std::string>( std::regex("(dg|ss|ois|lk|ok|wn|mb|th|ch|ec|oal|is|ck|ix|sser|ts|wb)ies$"), "$1ie" ),
    std::tuple<std::regex, std::string>( std::regex("(eau)x?$"), "$1" ),
    std::tuple<std::regex, std::string>( std::regex("(matr|append)ices$"), "$1ix" ),
    std::tuple<std::regex, std::string>( std::regex("(movie|twelve|abuse|e[mn]u)s$"), "$1" ),
    std::tuple<std::regex, std::string>( std::regex("(pe)(rson|ople)$"), "$1rson" ),
    std::tuple<std::regex, std::string>( std::regex("(seraph|cherub)im$"), "$1" ),
    std::tuple<std::regex, std::string>( std::regex("(ss)$"), "$1" ),
    std::tuple<std::regex, std::string>( std::regex("(test)(?:is|es)$"), "$1is" ),
    std::tuple<std::regex, std::string>( std::regex("(wi|kni|(?:after|half|high|low|mid|non|night|[^w]|^)li)ves$"), "$1fe" ),
    std::tuple<std::regex, std::string>( std::regex("(x|ch|ss|sh|zz|tto|go|cho|alias|[^aou]us|t[lm]as|gas|(?:her|at|gr)o|[aeiou]ris)(?:es)?$"), "$1" ),
    std::tuple<std::regex, std::string>( std::regex("b((?:tit)?m|l)ice$"), "$1ouse" ),
    std::tuple<std::regex, std::string>( std::regex("b(l|(?:neck|cross|hog|aun)?t|coll|faer|food|gen|goon|group|hipp|junk|vegg|(?:pork)?p|charl|calor|cut)ies$"), "$1ie" ),
    std::tuple<std::regex, std::string>( std::regex("b(mon|smil)ies$"), "$1ey" ),
    std::tuple<std::regex, std::string>( std::regex("ies$"), "y" ),
    std::tuple<std::regex, std::string>( std::regex("men$"), "man" ),
    std::tuple<std::regex, std::string>( std::regex("s$"), "" )
};

static const std::vector<std::tuple<std::regex, std::string>> PLURALIZATION_RULES = {
    std::tuple<std::regex, std::string>( std::regex("(?:(kni|wi|li)fe|(ar|l|ea|eo|oa|hoo)f)$"), "$1$2ves" ),
    std::tuple<std::regex, std::string>( std::regex("([^aeiou]ese)$"), "$1" ),
    std::tuple<std::regex, std::string>( std::regex("([^aeiouy]|qu)y$"), "$1ies" ),
    std::tuple<std::regex, std::string>( std::regex("([^ch][ieo][ln])ey$"), "$1ies" ),
    std::tuple<std::regex, std::string>( std::regex("([^l]ias|[aeiou]las|[ejzr]as|[iu]am)$"), "$1" ),
    std::tuple<std::regex, std::string>( std::regex("(agend|addend|millenni|dat|extrem|bacteri|desiderat|strat|candelabr|errat|ov|symposi|curricul|automat|quor)(?:a|um)$"), "$1a" ),
    std::tuple<std::regex, std::string>( std::regex("(alias|[^aou]us|t[lm]as|gas|ris)$"), "$1es" ),
    std::tuple<std::regex, std::string>( std::regex("(alumn|alg|vertebr)(?:a|ae)$"), "$1ae" ),
    std::tuple<std::regex, std::string>( std::regex("(alumn|syllab|vir|radi|nucle|fung|cact|stimul|termin|bacill|foc|uter|loc|strat)(?:us|i)$"), "$1i" ),
    std::tuple<std::regex, std::string>( std::regex("(apheli|hyperbat|periheli|asyndet|noumen|phenomen|criteri|organ|prolegomen|hedr|automat)(?:a|on)$"), "$1a" ),
    std::tuple<std::regex, std::string>( std::regex("(ax|test)is$"), "$1es" ),
    std::tuple<std::regex, std::string>( std::regex("(child)(?:ren)?$"), "$1ren" ),
    std::tuple<std::regex, std::string>( std::regex("(e[mn]u)s?$"), "$1s" ),
    std::tuple<std::regex, std::string>( std::regex("(her|at|gr)o$"), "$1oes" ),
    std::tuple<std::regex, std::string>( std::regex("(matr|cod|mur|sil|vert|ind|append)(?:ix|ex)$"), "$1ices" ),
    std::tuple<std::regex, std::string>( std::regex("(pe)(?:rson|ople)$"), "$1ople" ),
    std::tuple<std::regex, std::string>( std::regex("(seraph|cherub)(?:im)?$"), "$1im" ),
    std::tuple<std::regex, std::string>( std::regex("(x|ch|ss|sh|zz)$"), "$1es" ),
    std::tuple<std::regex, std::string>( std::regex("[^u0000-u007F]$"), "$0" ),
    std::tuple<std::regex, std::string>( std::regex("b((?:tit)?m|l)(?:ice|ouse)$"), "$1ice" ),
    std::tuple<std::regex, std::string>( std::regex("eaux$"), "$0" ),
    std::tuple<std::regex, std::string>( std::regex("m[ae]n$"), "men" ),
    std::tuple<std::regex, std::string>( std::regex("s?$"), "s" ),
    std::tuple<std::regex, std::string>( std::regex("sisi"), "ses" ),
    std::tuple<std::regex, std::string>( std::regex("thou"), "you" )
};
