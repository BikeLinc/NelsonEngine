#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace SimpleJson {

enum class Type {
	Null,
	Bool,
	Number,
	String,
	Array,
	Object
};

struct Value {
	Type type = Type::Null;
	bool boolValue = false;
	double numberValue = 0.0;
	std::string stringValue;
	std::vector<Value> arrayValue;
	std::map<std::string, Value> objectValue;

	static Value makeNull() {
		return Value{};
	}

	static Value makeBool(bool value) {
		Value v;
		v.type = Type::Bool;
		v.boolValue = value;
		return v;
	}

	static Value makeNumber(double value) {
		Value v;
		v.type = Type::Number;
		v.numberValue = value;
		return v;
	}

	static Value makeString(const std::string& value) {
		Value v;
		v.type = Type::String;
		v.stringValue = value;
		return v;
	}

	static Value makeArray() {
		Value v;
		v.type = Type::Array;
		return v;
	}

	static Value makeObject() {
		Value v;
		v.type = Type::Object;
		return v;
	}

	bool isObject() const { return type == Type::Object; }
	bool isArray() const { return type == Type::Array; }
	bool isString() const { return type == Type::String; }
	bool isNumber() const { return type == Type::Number; }
	bool isBool() const { return type == Type::Bool; }

	const Value* get(const std::string& key) const {
		if (!isObject()) {
			return nullptr;
		}
		auto it = objectValue.find(key);
		return it == objectValue.end() ? nullptr : &it->second;
	}

	std::string serialize(int indentSize = 2) const {
		std::ostringstream out;
		write(out, indentSize, 0);
		return out.str();
	}

private:
	static std::string escape(const std::string& in) {
		std::ostringstream out;
		for (char c : in) {
			switch (c) {
			case '\"': out << "\\\""; break;
			case '\\': out << "\\\\"; break;
			case '\n': out << "\\n"; break;
			case '\r': out << "\\r"; break;
			case '\t': out << "\\t"; break;
			default: out << c; break;
			}
		}
		return out.str();
	}

	void write(std::ostream& out, int indentSize, int depth) const {
		switch (type) {
		case Type::Null:
			out << "null";
			break;
		case Type::Bool:
			out << (boolValue ? "true" : "false");
			break;
		case Type::Number:
			out << std::setprecision(15) << numberValue;
			break;
		case Type::String:
			out << "\"" << escape(stringValue) << "\"";
			break;
		case Type::Array: {
			out << "[";
			if (!arrayValue.empty()) {
				out << "\n";
				for (size_t i = 0; i < arrayValue.size(); ++i) {
					out << std::string((depth + 1) * indentSize, ' ');
					arrayValue[i].write(out, indentSize, depth + 1);
					if (i + 1 < arrayValue.size()) {
						out << ",";
					}
					out << "\n";
				}
				out << std::string(depth * indentSize, ' ');
			}
			out << "]";
			break;
		}
		case Type::Object: {
			out << "{";
			if (!objectValue.empty()) {
				out << "\n";
				size_t i = 0;
				for (const auto& kv : objectValue) {
					out << std::string((depth + 1) * indentSize, ' ');
					out << "\"" << escape(kv.first) << "\": ";
					kv.second.write(out, indentSize, depth + 1);
					if (i + 1 < objectValue.size()) {
						out << ",";
					}
					out << "\n";
					++i;
				}
				out << std::string(depth * indentSize, ' ');
			}
			out << "}";
			break;
		}
		}
	}
};

class Parser {
public:
	explicit Parser(const std::string& source) : src(source) {}

	bool parse(Value& out, std::string* error = nullptr) {
		skipWhitespace();
		if (!parseValue(out, error)) {
			return false;
		}
		skipWhitespace();
		if (pos != src.size()) {
			setError("Unexpected trailing characters in JSON.", error);
			return false;
		}
		return true;
	}

private:
	const std::string& src;
	size_t pos = 0;

	void skipWhitespace() {
		while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) {
			++pos;
		}
	}

	bool consume(char expected) {
		if (pos < src.size() && src[pos] == expected) {
			++pos;
			return true;
		}
		return false;
	}

	bool parseValue(Value& out, std::string* error) {
		skipWhitespace();
		if (pos >= src.size()) {
			setError("Unexpected end of JSON.", error);
			return false;
		}

		char c = src[pos];
		if (c == '{') return parseObject(out, error);
		if (c == '[') return parseArray(out, error);
		if (c == '"') return parseStringValue(out, error);
		if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber(out, error);
		if (startsWith("true")) {
			pos += 4;
			out = Value::makeBool(true);
			return true;
		}
		if (startsWith("false")) {
			pos += 5;
			out = Value::makeBool(false);
			return true;
		}
		if (startsWith("null")) {
			pos += 4;
			out = Value::makeNull();
			return true;
		}

		setError("Invalid JSON value.", error);
		return false;
	}

	bool parseObject(Value& out, std::string* error) {
		if (!consume('{')) {
			setError("Expected '{'.", error);
			return false;
		}
		out = Value::makeObject();
		skipWhitespace();
		if (consume('}')) {
			return true;
		}

		while (true) {
			std::string key;
			if (!parseStringLiteral(key, error)) {
				return false;
			}
			skipWhitespace();
			if (!consume(':')) {
				setError("Expected ':' after object key.", error);
				return false;
			}
			Value value;
			if (!parseValue(value, error)) {
				return false;
			}
			out.objectValue[key] = value;
			skipWhitespace();
			if (consume('}')) {
				break;
			}
			if (!consume(',')) {
				setError("Expected ',' between object items.", error);
				return false;
			}
			skipWhitespace();
		}

		return true;
	}

	bool parseArray(Value& out, std::string* error) {
		if (!consume('[')) {
			setError("Expected '['.", error);
			return false;
		}
		out = Value::makeArray();
		skipWhitespace();
		if (consume(']')) {
			return true;
		}

		while (true) {
			Value value;
			if (!parseValue(value, error)) {
				return false;
			}
			out.arrayValue.push_back(value);
			skipWhitespace();
			if (consume(']')) {
				break;
			}
			if (!consume(',')) {
				setError("Expected ',' between array items.", error);
				return false;
			}
			skipWhitespace();
		}

		return true;
	}

	bool parseStringValue(Value& out, std::string* error) {
		std::string value;
		if (!parseStringLiteral(value, error)) {
			return false;
		}
		out = Value::makeString(value);
		return true;
	}

	bool parseStringLiteral(std::string& out, std::string* error) {
		if (!consume('"')) {
			setError("Expected string literal.", error);
			return false;
		}

		std::ostringstream value;
		while (pos < src.size()) {
			char c = src[pos++];
			if (c == '"') {
				out = value.str();
				return true;
			}
			if (c == '\\') {
				if (pos >= src.size()) {
					setError("Invalid escape sequence.", error);
					return false;
				}
				char esc = src[pos++];
				switch (esc) {
				case '"': value << '"'; break;
				case '\\': value << '\\'; break;
				case '/': value << '/'; break;
				case 'b': value << '\b'; break;
				case 'f': value << '\f'; break;
				case 'n': value << '\n'; break;
				case 'r': value << '\r'; break;
				case 't': value << '\t'; break;
				case 'u':
					if (pos + 4 > src.size()) {
						setError("Invalid unicode escape.", error);
						return false;
					}
					// Keep parser simple: consume codepoint and emit placeholder.
					pos += 4;
					value << '?';
					break;
				default:
					setError("Unsupported escape sequence.", error);
					return false;
				}
			} else {
				value << c;
			}
		}

		setError("Unterminated string literal.", error);
		return false;
	}

	bool parseNumber(Value& out, std::string* error) {
		const char* begin = src.c_str() + pos;
		char* endPtr = nullptr;
		double number = std::strtod(begin, &endPtr);
		if (endPtr == begin) {
			setError("Invalid number.", error);
			return false;
		}
		pos += static_cast<size_t>(endPtr - begin);
		out = Value::makeNumber(number);
		return true;
	}

	bool startsWith(const char* literal) const {
		size_t i = 0;
		while (literal[i] != '\0') {
			if (pos + i >= src.size() || src[pos + i] != literal[i]) {
				return false;
			}
			++i;
		}
		return true;
	}

	static void setError(const std::string& message, std::string* error) {
		if (error != nullptr) {
			*error = message;
		}
	}
};

inline bool parse(const std::string& source, Value& out, std::string* error = nullptr) {
	Parser parser(source);
	return parser.parse(out, error);
}

inline bool parseFile(const std::string& path, Value& out, std::string* error = nullptr) {
	std::ifstream in(path);
	if (!in.is_open()) {
		if (error != nullptr) {
			*error = "Failed to open JSON file: " + path;
		}
		return false;
	}
	std::stringstream buffer;
	buffer << in.rdbuf();
	return parse(buffer.str(), out, error);
}

inline bool writeFile(const std::string& path, const Value& value, std::string* error = nullptr) {
	std::ofstream out(path);
	if (!out.is_open()) {
		if (error != nullptr) {
			*error = "Failed to open JSON file for write: " + path;
		}
		return false;
	}
	out << value.serialize(2);
	out << "\n";
	return true;
}

} // namespace SimpleJson

#endif // SIMPLE_JSON_H
