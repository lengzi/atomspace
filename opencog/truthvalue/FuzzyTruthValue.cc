/*
 * opencog/truthvalue/FuzzyTruthValue.cc
 *
 * Copyright (C) 2002-2007 Novamente LLC
 * All Rights Reserved
 *
 * Written by Welter Silva <welter@vettalabs.com>
 *            Guilherme Lamacie

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <math.h>
#include <typeinfo>

#include <opencog/util/platform.h>
#include <opencog/util/exceptions.h>

#include "FuzzyTruthValue.h"

using namespace opencog;

FuzzyTruthValue::FuzzyTruthValue(strength_t m, count_t c)
	: TruthValue(FUZZY_TRUTH_VALUE)
{
    _value.resize(2);
    _value[MEAN] = m;
    _value[COUNT] = c;
}

FuzzyTruthValue::FuzzyTruthValue(const TruthValue& source)
	: TruthValue(FUZZY_TRUTH_VALUE)
{
    _value.resize(2);
    _value[MEAN] = source.getMean();
    _value[COUNT] = source.getCount();
}
FuzzyTruthValue::FuzzyTruthValue(FuzzyTruthValue const& source)
	: TruthValue(FUZZY_TRUTH_VALUE)
{
    _value.resize(2);
    _value[MEAN] = source.getMean();
    _value[COUNT] = source.getCount();
}

strength_t FuzzyTruthValue::getMean() const
{
    return _value[MEAN];
}

count_t FuzzyTruthValue::getCount() const
{
    return _value[COUNT];
}

confidence_t FuzzyTruthValue::getConfidence() const
{
    return countToConfidence(getCount());
}

// This is the merge formula appropriate for PLN.
TruthValuePtr FuzzyTruthValue::merge(TruthValuePtr other,
                                     const MergeCtrl& mc) const
{
    if (other->getType() != SIMPLE_TRUTH_VALUE) {
        throw RuntimeException(TRACE_INFO,
           "Don't know how to merge %s into a FuzzyTruthValue",
           typeid(*other).name());
    }

    if (other->getConfidence() > getConfidence()) {
        return other;
    }
    return std::dynamic_pointer_cast<const TruthValue>(shared_from_this());
}

std::string FuzzyTruthValue::toString(const std::string& indent) const
{
    char buf[1024];
    sprintf(buf, "(stv %f %f)",
            static_cast<float>(getMean()),
            static_cast<float>(getConfidence()));
    return buf;
}

bool FuzzyTruthValue::operator==(const ProtoAtom& rhs) const
{
    const FuzzyTruthValue *stv = dynamic_cast<const FuzzyTruthValue *>(&rhs);
    if (NULL == stv) return false;

#define FLOAT_ACCEPTABLE_MEAN_ERROR 0.000001
    if (FLOAT_ACCEPTABLE_MEAN_ERROR < fabs(getMean() - stv->getMean())) return false;

// Converting from confidence to count and back again using single-precision
// float is a real accuracy killer.  In particular, 2/802 = 0.002494 but
// converting back gives 800*0.002494/(1.0-0.002494) = 2.000188 and so
// comparison tests can only be accurate to about 0.000188 or
// thereabouts.
#define FLOAT_ACCEPTABLE_COUNT_ERROR 0.0002

    if (FLOAT_ACCEPTABLE_COUNT_ERROR < fabs(1.0 - (stv->getCount()/getCount()))) return false;
    return true;
}

count_t FuzzyTruthValue::confidenceToCount(confidence_t cf)
{
    // There are not quite 16 digits in double precision
    // not quite 7 in single-precision float
    cf = std::min(cf, 0.9999998f);
    return static_cast<count_t>(DEFAULT_K * cf / (1.0f - cf));
}

confidence_t FuzzyTruthValue::countToConfidence(count_t cn)
{
    return static_cast<confidence_t>(cn / (cn + DEFAULT_K));
}
