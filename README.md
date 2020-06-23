# Protobuf Json PrettyPrinter

This project contains a very small header-only library to pretty-print Protobuf messages to human-readable Json in C++.

Such functionality already exists in the protobuf library with
[<google/protobuf/util/json_util.h>](https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.util.json_util#JsonPrintOptions)

However, there are two major shortcomings in the standard json serialization:

1. It's not possible to override the json-serialization for specific message types to get idiomatic json.
For example, for time or date types, it's much more desirable to transform them into "YYYY-MM-DD" or "HH:MM" strings
1. It's not possible to specify that some types shall be printed on one line, resulting in poor readability.

# Example

Compare the [json output generated from this library](examples/examples1.json)
with the output [generated from google/protobuf/util/json_util.h](examples/example1.util_comparison.json)

# How to use it

Just copy ProtobufJsonPrinter.h into your project.

<pre><code>ProtobufJsonPrinter jsonPrint;

// Use your own function to print specific message types:
jsonPrint.overrideType&lt;your_protobuf::Date&gt; ( &printDate );
	
// For short types, prefer to print the on one line
jsonPrint.setTypeFlag( your_protobuf::PhoneNumber::descriptor(), ProtobufJsonPrinter::TypeFlag::OnOneLine );
	
jsonPrint.toJson (protoMessage, &str);

void printDate (const google::protobuf::Message &m, std::string *out) { ... }
</code></pre>

See [example1.cpp](examples/example1.cpp) for a full example

Note: The resulting Json from this library are meant for human consumption (e.g. debugging/analysis purposes)<br/>
It's __not__ meant to be used in production-grade code for the resulting messages to be used for machine processing.<br/>
i.E. it's hand-writing Json instead of using a library like rapidjson, in order to have no dependencies.
