#include <google/protobuf/message.h>
#include <cassert>
#include <functional>

/**
 * @brief Utility class to pretty-print Protobuf to Json.
 * 
 * Provides two advantages over google/protobuf/util/json_util.h:
 * 1. It's possible to provide custom serialization code for message-types
 * 2. Message-types can be configured to be printed on a single line, instead of line line per attribute to improve readability.
 */
class ProtobufJsonPrinter
{
	typedef std::function<void(const google::protobuf::Message &, std::string *)> Callback_t;
public:
	enum class TypeFlag {
		/// Print messages of this type on one-line, instead of one-line-per-attribute
		OnOneLine = 1,
	};
	
	/**
	 * @brief Serialize protobuf Message to json
	 */
	void toJson (const google::protobuf::Message &m, std::string *out)
	{
		this->newline = "\n";
		this->newlineLen = 1;
		this->indent = &indent_str[ strlen(indent_str) ];
		this->depth = 0;
		
		this->messageToJson(m, out);
	}
	
	template<class T>
	void overrideType (Callback_t cb)
	{
		Desc *d = T::descriptor();
		this->overrideType(d, std::move(cb));
	}
	
	/**
	 * @brief Provide override callback that is used to serialize a specific message type
	 */
	void overrideType (const google::protobuf::Descriptor *d, Callback_t cb)
	{
		TypeOverride t {d};
		auto it = std::lower_bound(overrides.begin(), overrides.end(), t);
		if (it == overrides.end() || it->d != d)
			it = overrides.insert(it, std::move(t));
		it->cb = std::move(cb);
		it->hasFunctor = true;
	}
	
	/**
	 * @brief Provide custom settings for specific message types
	 */
	void setTypeFlag (const google::protobuf::Descriptor *d, TypeFlag f)
	{
		TypeOverride t {d};
		auto it = std::lower_bound(overrides.begin(), overrides.end(), t);
		if (it == overrides.end() || it->d != d)
			it = overrides.insert(it, std::move(t));
		it->flags |= 1ULL << ((int)f);
		
	}
	
	void escapeString( const char * src, std::size_t len, std::string& target )
        {
		const char *e = src + len;
		for( ; src != e; ++src )
		{
			switch( *src ) {
			case '\\':  target.append( "\\\\" ); break;
			case '"':  target.append( "\\\"" ); break;
			case '\n': target.append( "\\n" ); break;
			case '\t': target.append( "\\t" ); break;
			case '\r':  target.append( "\\r" ); break;
			case '\b':  target.append( "\\b" ); break;
			case '\f':  target.append( "\\f" ); break;
			default:   target.push_back( *src ); break;
			}
		}
	}
	
private:
	template<bool IsRepeated>
	inline void handleField (google::protobuf::FieldDescriptor::Type tp, const google::protobuf::Reflection *refl,
			const google::protobuf::FieldDescriptor *f, const google::protobuf::Message &m, int index, std::string *out);
	
	inline void messageToJson (const google::protobuf::Message &m, std::string *out);
	
	template<class T, bool IsRepeated,
		T (google::protobuf::Reflection::*repeatedFunc) (const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field, int index) const,
		T (google::protobuf::Reflection::*singularFunc) (const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field) const
	>
	static void handlePrimField (const google::protobuf::Message &m, const google::protobuf::Reflection *refl,
				const google::protobuf::FieldDescriptor *f, int index, std::string *out, const char *format)
	{
		char buf[32];
		T d = IsRepeated ? (refl->*repeatedFunc) (m, f, index) : (refl->*singularFunc) (m, f);
		sprintf(buf, format, d);
		out->append(buf);
	}
	
private:
	typedef const google::protobuf::Descriptor Desc;
	std::string tmpFieldStr;
	static const int MaxDepth = 30;
	static constexpr const char * const indent_str =    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\0";
	static constexpr const char * const no_indent_str = "                                                                \0";
	const char *indent;
	const char *newline;
	std::uint8_t newlineLen;
	std::uint16_t depth;
	
	struct TypeOverride
	{
		TypeOverride (Desc *x) : d{x} {}
		
		Desc *d;
		Callback_t cb;
		
		bool hasFunctor = false;
		std::uint16_t flags = 0;
		
		bool operator< (const TypeOverride &o) const { return d < o.d; }
	};
	std::vector<TypeOverride> overrides;
};

// Impl

template<bool IsRepeated>
void ProtobufJsonPrinter::handleField (google::protobuf::FieldDescriptor::Type tp, const google::protobuf::Reflection *refl,
			const google::protobuf::FieldDescriptor *f, const google::protobuf::Message &m, int index, std::string *out)
{
	using namespace google::protobuf;
	
	
	switch(tp)
	{
	case FieldDescriptor::TYPE_DOUBLE:
		handlePrimField<double, IsRepeated, &Reflection::GetRepeatedDouble, &Reflection::GetDouble> (m, refl, f, index, out, "%d"); break;
	case FieldDescriptor::TYPE_INT32:
		handlePrimField<std::int32_t, IsRepeated, &Reflection::GetRepeatedInt32, &Reflection::GetInt32> (m, refl, f, index, out, "%i"); break;
	case FieldDescriptor::TYPE_INT64:
		handlePrimField<std::int64_t, IsRepeated, &Reflection::GetRepeatedInt64, &Reflection::GetInt64> (m, refl, f, index, out, "%li"); break;
	case FieldDescriptor::TYPE_FLOAT:
		handlePrimField<float, IsRepeated, &Reflection::GetRepeatedFloat, &Reflection::GetFloat> (m, refl, f, index, out, "%f"); break;
	case FieldDescriptor::TYPE_UINT32:
		handlePrimField<std::uint32_t, IsRepeated, &Reflection::GetRepeatedUInt32, &Reflection::GetUInt32> (m, refl, f, index, out, "%u"); break;
	case FieldDescriptor::TYPE_UINT64:
		handlePrimField<std::uint64_t, IsRepeated, &Reflection::GetRepeatedUInt64, &Reflection::GetUInt64> (m, refl, f, index, out, "%lu"); break;
	
	case FieldDescriptor::TYPE_BOOL: {
		bool d = IsRepeated ? refl->GetRepeatedBool (m, f, index) : refl->GetBool(m, f);
		out->append(d ? "true" : "false");
		break;
	}
	case FieldDescriptor::TYPE_STRING: {
		out->push_back('"');
		const std::string &d = refl->GetStringReference(m, f, &this->tmpFieldStr);
		escapeString(d.data(), d.size(), *out);
		out->push_back('"');
		break;
	}
	case FieldDescriptor::TYPE_ENUM: {
		const EnumValueDescriptor *evd = refl->GetEnum(m, f);
		out->push_back('"');
		out->append(evd->name());
		out->push_back('"');
		break;
	}
	
	case FieldDescriptor::TYPE_MESSAGE: {
		const Message &cm = IsRepeated ? refl->GetRepeatedMessage(m, f, index) : refl->GetMessage(m, f);
		auto cmp = [] (const TypeOverride &a, const Descriptor *b) { return a.d < b; };
		
		auto d = cm.GetDescriptor();
		auto it = std::lower_bound( overrides.begin(), overrides.end(), d, cmp);
		const bool hasOverride = it != overrides.end() && it->d == d;
		if (hasOverride && it->hasFunctor )
			it->cb( cm, out );
		else {
			const char *indent_restore = this->indent;
			const char *newline_restore = this->newline;
			const std::uint8_t newlineLen_restore = this->newlineLen;
			
			if (hasOverride && (it->flags >> ((int)TypeFlag::OnOneLine)) & 1) {
				this->indent = &no_indent_str[ strlen(no_indent_str) ];
				this->newline = "";
				this->newlineLen = 0;
			}
			
			messageToJson(cm, out);
			
			// Restore is only needed for OnOneLine case.
			this->indent = indent_restore;
			this->newline = newline_restore;
			this->newlineLen = newlineLen_restore;
		}
		break;
	}
	default:
		throw std::runtime_error("Unknown type for Json transformation: " + std::to_string( (int)tp ));
		break;
	}
}

void ProtobufJsonPrinter::messageToJson (const google::protobuf::Message &m, std::string *out)
{
	if (++depth > MaxDepth)
		throw std::runtime_error("Exceeded maximum depth of Json serialization");
	
	using namespace google::protobuf;
	auto *desc = m.GetDescriptor();
	assert(desc);
	const Reflection *refl = m.GetReflection();
	assert(refl);
	
	out->append("{");
	out->append(newline);
	--indent;
	
	bool wroteElements = false;
	for (unsigned i = 0, n = desc->field_count(); i < n; ++i)
	{
		const FieldDescriptor *f = desc->field(i);
		assert(f);
		if (!f->is_repeated() && !refl->HasField(m, f))
			continue;
		
		const FieldDescriptor::Type tp = f->type();
		
		out->append(indent);
		out->append("\"");
		const std::string &f_name = f->json_name();
		escapeString(f_name.data(), f_name.size(), *out);
		out->append("\": ");
		
		if (f->is_repeated()) {
			out->append("[");
			int count = refl->FieldSize(m, f);
			if (count > 0) 
				out->append("\n");
			for (int j = 0; j < count; ++j) {
				if (tp == FieldDescriptor::TYPE_MESSAGE) {
					--indent;
					out->append(indent);
				}
				handleField<true> (tp, refl, f, m, j, out);
				if (j + 1 != count)
					out->append(", ");
				
				if (tp == FieldDescriptor::TYPE_MESSAGE) {
					++indent;
					if (j + 1 != count)
						out->append("\n");
				}
			}
			if (count > 0)  {
				out->append("\n");
				out->append(indent);
				out->append("]");
			}
			else
				out->append("]");
		}
		else {
		
			handleField<false> (tp, refl, f, m, ~0, out);
		}
		
		wroteElements = true;
		out->push_back(',');
		out->append(newline);
	}
	if (wroteElements) {
		out->resize( out->size() - (1 + newlineLen) ); // Remove last comma
		out->append(newline);
	}
	
	++indent;
	out->append(indent);
	out->append("}");
	--depth;
}
