#include "mod_json.h"

namespace
{
using JSON = nlohmann::json;

template <typename ENUM, size_t COUNT>
using NAME_TABLE = std::array<std::pair<std::string_view, ENUM>, COUNT>;

constexpr const uint32_t MAX_COUNT = std::numeric_limits<uint32_t>::max();
constexpr const uint32_t MAX_DIGITS = 32;
constexpr const int64_t MAX_NUMBER = 1000000000;

constexpr const NAME_TABLE<MOD_KIND, 4> KIND_NAMES = {{
    {"insert", MOD_KIND::insert},
    {"delete", MOD_KIND::remove},
    {"replace", MOD_KIND::replace},
    {"format", MOD_KIND::format},
}};

constexpr const NAME_TABLE<MOD_PART, 3> PART_NAMES = {{
    {"name", MOD_PART::name},
    {"ext", MOD_PART::ext},
    {"full", MOD_PART::full},
}};

constexpr const NAME_TABLE<POSITION_TYPE, 12> POSITION_NAMES = {{
    {"start", POSITION_TYPE::start},
    {"end", POSITION_TYPE::end},
    {"after_nth", POSITION_TYPE::after_nth},
    {"before_nth_from_end", POSITION_TYPE::before_nth_from_end},
    {"before_text", POSITION_TYPE::before_text},
    {"after_text", POSITION_TYPE::after_text},
    {"all", POSITION_TYPE::all},
    {"first_n", POSITION_TYPE::first_n},
    {"last_n", POSITION_TYPE::last_n},
    {"from_nth", POSITION_TYPE::from_nth},
    {"text", POSITION_TYPE::text},
    {"between", POSITION_TYPE::between},
}};

constexpr const NAME_TABLE<OCCURRENCE, 5> OCCURRENCE_NAMES = {{
    {"first", OCCURRENCE::first},
    {"last", OCCURRENCE::last},
    {"nth", OCCURRENCE::nth},
    {"nth_from_end", OCCURRENCE::nth_from_end},
    {"all", OCCURRENCE::all},
}};

constexpr const NAME_TABLE<TOKEN_TYPE, 6> TOKEN_NAMES = {{
    {"number", TOKEN_TYPE::number},
    {"original_name", TOKEN_TYPE::original_name},
    {"modified_date", TOKEN_TYPE::modified_date},
    {"created_date", TOKEN_TYPE::created_date},
    {"size", TOKEN_TYPE::size},
    {"parent_folder", TOKEN_TYPE::parent_folder},
}};

constexpr const NAME_TABLE<FORMAT_TYPE, 5> FORMAT_NAMES = {{
    {"upper", FORMAT_TYPE::upper},
    {"lower", FORMAT_TYPE::lower},
    {"title", FORMAT_TYPE::title},
    {"sentence", FORMAT_TYPE::sentence},
    {"pad_digits", FORMAT_TYPE::pad_digits},
}};

constexpr const NAME_TABLE<MOD_SCOPE, 2> SCOPE_NAMES = {{
    {"all", MOD_SCOPE::all},
    {"items", MOD_SCOPE::items},
}};

[[noreturn]] void fail(std::string_view message, std::string_view key)
{
    throw MOD_JSON_ERROR(std::string(message) + ": " + std::string(key));
}

template <typename ENUM, size_t COUNT>
ENUM enum_from(const NAME_TABLE<ENUM, COUNT> &table, const std::string &name,
    std::string_view key)
{
    for (const auto &[text, value] : table) {
        if (text == name) {
            return value;
        }
    }
    fail("unknown value for " + std::string(key), name);
}

template <typename ENUM, size_t COUNT>
std::string enum_name(const NAME_TABLE<ENUM, COUNT> &table, ENUM value)
{
    for (const auto &[text, item] : table) {
        if (item == value) {
            return std::string(text);
        }
    }
    throw std::logic_error("enum value has no name");
}

const JSON *find(const JSON &object, std::string_view key)
{
    const auto value = object.find(key);
    return value == object.end() ? nullptr : &*value;
}

const JSON &require(const JSON &object, std::string_view key)
{
    const JSON *value = find(object, key);
    if (value == nullptr) {
        fail("missing field", key);
    }
    return *value;
}

const JSON &require_object(const JSON &value, std::string_view key)
{
    if (!value.is_object()) {
        fail("field must be an object", key);
    }
    return value;
}

std::string read_string(const JSON &value, std::string_view key)
{
    if (!value.is_string()) {
        fail("field must be a string", key);
    }
    return value.get<std::string>();
}

std::u32string read_text(const JSON &value, std::string_view key)
{
    const std::optional<std::u32string> text = UTF::decode_utf8(read_string(value, key));
    if (!text) {
        fail("field has invalid UTF-8", key);
    }
    return *text;
}

bool read_bool(const JSON &object, std::string_view key, bool fallback)
{
    const JSON *value = find(object, key);
    if (value == nullptr) {
        return fallback;
    }
    if (!value->is_boolean()) {
        fail("field must be a boolean", key);
    }
    return value->get<bool>();
}

uint64_t read_unsigned(const JSON &value, std::string_view key, uint64_t minimum,
    uint64_t maximum)
{
    if (!value.is_number_unsigned()) {
        fail("field must be a non-negative integer", key);
    }
    const uint64_t number = value.get<uint64_t>();
    if (number < minimum || number > maximum) {
        fail("field is out of range", key);
    }
    return number;
}

uint32_t read_count(const JSON &object, std::string_view key, uint32_t minimum, uint32_t maximum)
{
    return static_cast<uint32_t>(read_unsigned(require(object, key), key, minimum, maximum));
}

uint32_t read_optional_count(const JSON &object, std::string_view key, uint32_t fallback,
    uint32_t minimum, uint32_t maximum)
{
    const JSON *value = find(object, key);
    if (value == nullptr) {
        return fallback;
    }
    return static_cast<uint32_t>(read_unsigned(*value, key, minimum, maximum));
}

int64_t read_number(const JSON &object, std::string_view key, int64_t fallback)
{
    const JSON *value = find(object, key);
    if (value == nullptr) {
        return fallback;
    }
    int64_t number = 0;
    if (value->is_number_unsigned()) {
        const uint64_t positive = value->get<uint64_t>();
        if (positive > static_cast<uint64_t>(MAX_NUMBER)) {
            fail("field is out of range", key);
        }
        number = static_cast<int64_t>(positive);
    } else if (value->is_number_integer()) {
        number = value->get<int64_t>();
    } else {
        fail("field must be an integer", key);
    }
    if (number < -MAX_NUMBER || number > MAX_NUMBER) {
        fail("field is out of range", key);
    }
    return number;
}

bool is_insert_position(POSITION_TYPE type)
{
    switch (type) {
    case POSITION_TYPE::start:
    case POSITION_TYPE::end:
    case POSITION_TYPE::after_nth:
    case POSITION_TYPE::before_nth_from_end:
    case POSITION_TYPE::before_text:
    case POSITION_TYPE::after_text:
        return true;
    default:
        return false;
    }
}

bool uses_occurrence_n(OCCURRENCE occurrence)
{
    return occurrence == OCCURRENCE::nth || occurrence == OCCURRENCE::nth_from_end;
}

bool has_value(MOD_KIND kind)
{
    return kind == MOD_KIND::insert || kind == MOD_KIND::replace;
}

void read_occurrence(const JSON &object, POSITION &position, bool allow_all)
{
    const JSON *value = find(object, "occurrence");
    if (value != nullptr) {
        position.occurrence = enum_from(OCCURRENCE_NAMES, read_string(*value, "occurrence"),
            "occurrence");
    }
    if (!allow_all && position.occurrence == OCCURRENCE::all) {
        fail("occurrence is not allowed", "all");
    }
    if (uses_occurrence_n(position.occurrence)) {
        position.occurrence_n = read_count(object, "occurrence_n", 1, MAX_COUNT);
    }
}

POSITION parse_position(const JSON &value, MOD_KIND kind)
{
    const JSON &object = require_object(value, "position");
    POSITION position;
    position.type = enum_from(POSITION_NAMES, read_string(require(object, "type"), "type"),
        "position type");
    if (is_insert_position(position.type) != (kind == MOD_KIND::insert)) {
        fail("position type does not match kind", read_string(object["type"], "type"));
    }
    switch (position.type) {
    case POSITION_TYPE::start:
    case POSITION_TYPE::end:
    case POSITION_TYPE::all:
        break;
    case POSITION_TYPE::after_nth:
    case POSITION_TYPE::before_nth_from_end:
    case POSITION_TYPE::first_n:
    case POSITION_TYPE::last_n:
        position.n = read_count(object, "n", 1, MAX_COUNT);
        break;
    case POSITION_TYPE::from_nth:
        position.n = read_count(object, "n", 1, MAX_COUNT);
        position.count = read_count(object, "count", 1, MAX_COUNT);
        break;
    case POSITION_TYPE::before_text:
    case POSITION_TYPE::after_text:
    case POSITION_TYPE::text:
        position.text = read_text(require(object, "text"), "text");
        position.case_sensitive = read_bool(object, "case_sensitive", false);
        read_occurrence(object, position, true);
        break;
    case POSITION_TYPE::between:
        position.from_beginning = read_bool(object, "from_beginning", false);
        if (!position.from_beginning) {
            position.start_text = read_text(require(object, "start_text"), "start_text");
        }
        position.to_end = read_bool(object, "to_end", false);
        if (!position.to_end) {
            position.end_text = read_text(require(object, "end_text"), "end_text");
        }
        position.case_sensitive = read_bool(object, "case_sensitive", false);
        position.include_delimiters = read_bool(object, "include_delimiters", false);
        read_occurrence(object, position, false);
        break;
    }
    return position;
}

std::vector<VALUE_PIECE> parse_value(const JSON &value)
{
    if (!value.is_array()) {
        fail("field must be an array", "value");
    }
    std::vector<VALUE_PIECE> pieces;
    for (const JSON &piece : value) {
        const JSON &object = require_object(piece, "value piece");
        const JSON *text = find(object, "text");
        const JSON *token = find(object, "token");
        if ((text == nullptr) == (token == nullptr)) {
            fail("value piece needs exactly one of", "text, token");
        }
        if (text != nullptr) {
            pieces.emplace_back(read_text(*text, "text"));
        } else {
            pieces.emplace_back(enum_from(TOKEN_NAMES, read_string(*token, "token"), "token"));
        }
    }
    return pieces;
}

void parse_tokens(const JSON &root, MODIFICATION &mod)
{
    const JSON *tokens = find(root, "tokens");
    if (tokens == nullptr) {
        return;
    }
    require_object(*tokens, "tokens");
    const JSON *number = find(*tokens, "number");
    if (number != nullptr) {
        require_object(*number, "number");
        mod.number.start = read_number(*number, "start", 1);
        mod.number.step = read_number(*number, "step", 1);
        mod.number.digits = read_optional_count(*number, "digits", 1, 1, MAX_DIGITS);
    }
}

FORMAT_SETTINGS parse_format(const JSON &value)
{
    const JSON &object = require_object(value, "format");
    FORMAT_SETTINGS format;
    format.type = enum_from(FORMAT_NAMES, read_string(require(object, "type"), "type"),
        "format type");
    if (format.type == FORMAT_TYPE::pad_digits) {
        format.digits = read_count(object, "digits", 1, MAX_DIGITS);
    }
    return format;
}

std::vector<uint64_t> parse_targets(const JSON &root)
{
    std::vector<uint64_t> targets;
    const JSON *value = find(root, "targets");
    if (value == nullptr) {
        return targets;
    }
    if (!value->is_array()) {
        fail("field must be an array", "targets");
    }
    for (const JSON &target : *value) {
        targets.push_back(read_unsigned(target, "targets", 1,
            std::numeric_limits<uint64_t>::max()));
    }
    std::sort(targets.begin(), targets.end());
    targets.erase(std::unique(targets.begin(), targets.end()), targets.end());
    return targets;
}

JSON position_json(const POSITION &position)
{
    JSON object = JSON::object();
    object["type"] = enum_name(POSITION_NAMES, position.type);
    switch (position.type) {
    case POSITION_TYPE::start:
    case POSITION_TYPE::end:
    case POSITION_TYPE::all:
        break;
    case POSITION_TYPE::after_nth:
    case POSITION_TYPE::before_nth_from_end:
    case POSITION_TYPE::first_n:
    case POSITION_TYPE::last_n:
        object["n"] = position.n;
        break;
    case POSITION_TYPE::from_nth:
        object["n"] = position.n;
        object["count"] = position.count;
        break;
    case POSITION_TYPE::before_text:
    case POSITION_TYPE::after_text:
    case POSITION_TYPE::text:
        object["text"] = UTF::encode_utf8(position.text);
        object["case_sensitive"] = position.case_sensitive;
        object["occurrence"] = enum_name(OCCURRENCE_NAMES, position.occurrence);
        break;
    case POSITION_TYPE::between:
        object["from_beginning"] = position.from_beginning;
        if (!position.from_beginning) {
            object["start_text"] = UTF::encode_utf8(position.start_text);
        }
        object["to_end"] = position.to_end;
        if (!position.to_end) {
            object["end_text"] = UTF::encode_utf8(position.end_text);
        }
        object["case_sensitive"] = position.case_sensitive;
        object["include_delimiters"] = position.include_delimiters;
        object["occurrence"] = enum_name(OCCURRENCE_NAMES, position.occurrence);
        break;
    }
    if (object.contains("occurrence") && uses_occurrence_n(position.occurrence)) {
        object["occurrence_n"] = position.occurrence_n;
    }
    return object;
}
}

MODIFICATION MOD_JSON::parse(std::string_view text)
{
    JSON root;
    try {
        root = JSON::parse(text.begin(), text.end());
    } catch (const JSON::exception &error) {
        throw MOD_JSON_ERROR(std::string("invalid JSON: ") + error.what());
    }
    require_object(root, "modification");

    MODIFICATION mod;
    mod.kind = enum_from(KIND_NAMES, read_string(require(root, "kind"), "kind"), "kind");
    const JSON *part = find(root, "part");
    if (part != nullptr) {
        mod.part = enum_from(PART_NAMES, read_string(*part, "part"), "part");
    }
    mod.position = parse_position(require(root, "position"), mod.kind);
    if (has_value(mod.kind)) {
        mod.value = parse_value(require(root, "value"));
        parse_tokens(root, mod);
    }
    if (mod.kind == MOD_KIND::format) {
        mod.format = parse_format(require(root, "format"));
    }
    const JSON *scope = find(root, "scope");
    if (scope != nullptr) {
        mod.scope = enum_from(SCOPE_NAMES, read_string(*scope, "scope"), "scope");
    }
    if (mod.scope == MOD_SCOPE::items) {
        mod.targets = parse_targets(root);
    }
    mod.direct = read_bool(root, "direct", false);
    return mod;
}

std::string MOD_JSON::to_json(const MODIFICATION &mod)
{
    JSON root = JSON::object();
    root["kind"] = enum_name(KIND_NAMES, mod.kind);
    root["part"] = enum_name(PART_NAMES, mod.part);
    root["position"] = position_json(mod.position);
    if (has_value(mod.kind)) {
        JSON value = JSON::array();
        for (const VALUE_PIECE &piece : mod.value) {
            JSON object = JSON::object();
            if (const std::u32string *text = std::get_if<std::u32string>(&piece)) {
                object["text"] = UTF::encode_utf8(*text);
            } else {
                object["token"] = enum_name(TOKEN_NAMES, std::get<TOKEN_TYPE>(piece));
            }
            value.push_back(std::move(object));
        }
        root["value"] = std::move(value);
        JSON number = JSON::object();
        number["start"] = mod.number.start;
        number["step"] = mod.number.step;
        number["digits"] = mod.number.digits;
        root["tokens"]["number"] = std::move(number);
    }
    if (mod.kind == MOD_KIND::format) {
        root["format"]["type"] = enum_name(FORMAT_NAMES, mod.format.type);
        if (mod.format.type == FORMAT_TYPE::pad_digits) {
            root["format"]["digits"] = mod.format.digits;
        }
    }
    root["scope"] = enum_name(SCOPE_NAMES, mod.scope);
    if (mod.scope == MOD_SCOPE::items) {
        root["targets"] = mod.targets;
    }
    root["direct"] = mod.direct;
    return root.dump();
}
