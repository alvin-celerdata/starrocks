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

#include <utility>

#include "column/datum_convert.h"
#include "types/logical_type.h"
#include "types/type_traits.h"

namespace starrocks {

template <LogicalType TYPE>
static std::string datum_to_string_with_type_info(TypeInfo* type_info, const Datum& datum) {
    using CppType = typename CppTypeTraits<TYPE>::CppType;
    auto value = datum.template get<CppType>();
    return type_info->to_string(&value);
}

StatusOr<std::string> datum_to_string_checked(TypeInfo* type_info, const Datum& datum) {
    if (type_info == nullptr) {
        return Status::InvalidArgument("type_info is null");
    }
    if (datum.is_null()) {
        return "null";
    }

    switch (type_info->type()) {
    case TYPE_BOOLEAN:
        // Keep historical behavior: bool prints as "0"/"1".
        return datum_to_string_with_type_info<TYPE_TINYINT>(type_info, datum);
    case TYPE_VARBINARY:
    case TYPE_CHAR:
    case TYPE_VARCHAR:
        return datum_to_string_with_type_info<TYPE_VARCHAR>(type_info, datum);
    case TYPE_TINYINT:
        return datum_to_string_with_type_info<TYPE_TINYINT>(type_info, datum);
    case TYPE_SMALLINT:
        return datum_to_string_with_type_info<TYPE_SMALLINT>(type_info, datum);
    case TYPE_BIGINT:
        return datum_to_string_with_type_info<TYPE_BIGINT>(type_info, datum);
    case TYPE_LARGEINT:
        return datum_to_string_with_type_info<TYPE_LARGEINT>(type_info, datum);
    case TYPE_INT:
        return datum_to_string_with_type_info<TYPE_INT>(type_info, datum);
    case TYPE_INT256:
        return datum_to_string_with_type_info<TYPE_INT256>(type_info, datum);
    case TYPE_DATE_V1:
        return datum_to_string_with_type_info<TYPE_DATE_V1>(type_info, datum);
    case TYPE_DATE:
        return datum_to_string_with_type_info<TYPE_DATE>(type_info, datum);
    case TYPE_DATETIME_V1:
        return datum_to_string_with_type_info<TYPE_DATETIME_V1>(type_info, datum);
    case TYPE_DATETIME:
        return datum_to_string_with_type_info<TYPE_DATETIME>(type_info, datum);
    case TYPE_DECIMAL:
        return datum_to_string_with_type_info<TYPE_DECIMAL>(type_info, datum);
    case TYPE_DECIMALV2:
        return datum_to_string_with_type_info<TYPE_DECIMALV2>(type_info, datum);
    case TYPE_DECIMAL32:
        return datum_to_string_with_type_info<TYPE_DECIMAL32>(type_info, datum);
    case TYPE_DECIMAL64:
        return datum_to_string_with_type_info<TYPE_DECIMAL64>(type_info, datum);
    case TYPE_DECIMAL128:
        return datum_to_string_with_type_info<TYPE_DECIMAL128>(type_info, datum);
    case TYPE_DECIMAL256:
        return datum_to_string_with_type_info<TYPE_DECIMAL256>(type_info, datum);
    case TYPE_FLOAT:
        return datum_to_string_with_type_info<TYPE_FLOAT>(type_info, datum);
    case TYPE_JSON:
        return datum_to_string_with_type_info<TYPE_JSON>(type_info, datum);
    case TYPE_DOUBLE:
        return datum_to_string_with_type_info<TYPE_DOUBLE>(type_info, datum);
    default:
        return Status::NotSupported("Type not supported: " + type_to_string(type_info->type()));
    }
}

std::string datum_to_string(TypeInfo* type_info, const Datum& datum) {
    auto result = datum_to_string_checked(type_info, datum);
    if (!result.ok()) {
        return "";
    }
    return std::move(result).value();
}

} // namespace starrocks
