// Copyright 2021-present StarRocks, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "column/datum_convert.h"

#include <cstring>

#include "gutil/strings/substitute.h"
#include "runtime/mem_pool.h"
#include "storage/olap_type_infra.h"
#include "types/type_traits.h"

namespace starrocks {

using strings::Substitute;

template <LogicalType TYPE>
Status datum_from_string(TypeInfo* type_info, Datum* dst, const std::string& str) {
    typename CppTypeTraits<TYPE>::CppType value;
    RETURN_IF_ERROR(type_info->from_string(&value, str));
    dst->set(value);
    return Status::OK();
}

Status datum_from_string(TypeInfo* type_info, Datum* dst, const std::string& str, MemPool* mem_pool) {
    const auto type = type_info->type();
    switch (type) {
#define M(type) \
    case type:  \
        return datum_from_string<type>(type_info, dst, str);

        APPLY_FOR_TYPE_INTEGER(M)
        APPLY_FOR_TYPE_TIME(M)
        APPLY_FOR_TYPE_DECIMAL(M)
        M(TYPE_FLOAT)
        M(TYPE_DOUBLE)
#undef M
    case TYPE_BOOLEAN: {
        bool v;
        RETURN_IF_ERROR(type_info->from_string(&v, str));
        dst->set_int8(v);
        return Status::OK();
    }
        /* Type need memory allocated */
    case TYPE_VARBINARY:
    case TYPE_CHAR:
    case TYPE_VARCHAR: {
        /* Type need memory allocated */
        Slice slice;
        slice.size = str.size();
        if (mem_pool == nullptr) {
            slice.data = (char*)str.data();
        } else {
            slice.data = reinterpret_cast<char*>(mem_pool->allocate(slice.size));
            RETURN_IF_UNLIKELY_NULL(slice.data, Status::MemoryAllocFailed("alloc mem for varchar field failed"));
            memcpy(slice.data, str.data(), slice.size);
        }
        // If type is TYPE_CHAR, strip its tailing '\0'
        if (type == TYPE_CHAR) {
            slice.size = strnlen(slice.data, slice.size);
        }
        dst->set_slice(slice);
        break;
    }
    default:
        return Status::NotSupported(Substitute("Type $0 not supported", type));
    }

    return Status::OK();
}
} // namespace starrocks
