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

#ifndef Utils_h
#define Utils_h 1

#include <stdio.h>
#include <set>
#include <string>
#include <vector>

#include "config.h"
#include "execute.h"
#include "streams.h"

#undef MAX
#undef MIN
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

#define Arraysize(x) (sizeof(x) / sizeof(*x))

extern int verbcasecmp(const char *verb, const char *word);

extern unsigned str_hash(const char *);

extern void complex_free_var(Var);
extern Var complex_var_ref(Var);
extern Var complex_var_dup(Var);
extern int var_refcount(Var);

extern void aux_free(Var);

static inline void
free_var(Var v)
{
    if (v.is_complex())
	complex_free_var(v);
}

static inline Var
var_ref(Var v)
{
    if (v.is_complex())
	return complex_var_ref(v);
    else
	return v;
}

static inline Var
var_dup(Var v)
{
    if (v.is_complex())
	return complex_var_dup(v);
    else
	return v;
}


namespace ts {
    namespace var {

        static inline Var make_string_var(std::string s) { return str_dup_to_var(s.c_str()); }

        static inline std::vector<std::string> 
        var_to_vector(Var v) {
            std::vector<std::string> results;
            switch (v.type) {
                case TYPE_LIST:
                    for (int i=1; i <= v.v.list[0].v.num;i++) {
                        if (v.v.list[i].type != TYPE_STR) continue;
                        results.emplace_back(std::string(v.v.list[i].v.str));
                    }
                    break;
                case TYPE_STR:
                    results.emplace_back(std::string(v.v.str));
                    break;
            }
            return results;
        }

        template <typename T>
        static inline Var vec_to_list(std::vector<T> v) {
            auto n  = v.size();
            Var ret = new_list(n);

            if(n==0) return ret;

            auto i = 0;

            if(std::is_same<T, Objid>::value) {
                for(T& x: v) {
                    if(!valid(x)) continue;
                    ret.v.list[++i] = Var::new_obj(x);
                }
            }

            return ret;
        }
    }

    namespace utils {
        template <typename T>
        static inline std::vector<T> merge_unique(std::vector<T> v1, std::vector<T> v2) {
            auto sz = v1.size() + v2.size();

            std::vector<T> v, r;

            v.reserve(sz);
            r.reserve(sz);

            v.insert(v.end(), v1.begin(), v1.end());
            v.insert(v.end(), v2.begin(), v2.end());

            std::set<T> a;
            for (auto & element : v) {
                if(a.count(element)) continue;
                a.insert(element);
                r.emplace_back(element);
            }

            r.shrink_to_fit();

            return r;
        }
    }
}

extern int is_true(Var v);
extern int compare(Var lhs, Var rhs, int case_matters);
extern int equality(Var lhs, Var rhs, int case_matters);

extern void stream_add_strsub(Stream *, const char *, const char *, const char *, int);
extern int strindex(const char *, int, const char *, int, int);
extern int strrindex(const char *, int, const char *, int, int);

extern const char *strtr(const char *, int, const char *, int, const char *, int, int);

extern Var get_system_property(const char *);
extern Objid get_system_object(const char *);

extern int value_bytes(Var);

extern void stream_add_raw_bytes_to_clean(Stream *, const char *buffer, int buflen);
extern const char *raw_bytes_to_clean(const char *buffer, int buflen);
extern const char *clean_to_raw_bytes(const char *binary, int *rawlen);

extern void stream_add_raw_bytes_to_binary(Stream *, const char *buffer, int buflen);
extern const char *raw_bytes_to_binary(const char *buffer, int buflen);
extern const char *binary_to_raw_bytes(const char *binary, int *rawlen);

extern Var anonymizing_var_ref(Var v, Objid progr);
				/* To be used in places where you
				 * need to copy (via `var_ref()') a
				 * value and also need to ensure that
				 * _if_ it's an anonymous object it
				 * remains anonymous for
				 * non-wizards/owners.
				 */

#endif
