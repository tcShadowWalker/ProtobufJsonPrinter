#include "ProtobufJsonPrinter.h"
#include "examples/example1.pb.h"
#include <iostream>
#include <fstream>

namespace td = Example::Api;

void printTimestamp (const google::protobuf::Message &m, std::string *out)
{
	const auto *d = static_cast<const google::protobuf::Timestamp*> (&m);
	char buf[32];
	std::tm tm;
	const time_t t = d->seconds();
	gmtime_r( &t, &tm );
	int r = strftime(buf, sizeof(buf), "\"%Y-%m-%dT%H:%M:%SZ\"", &tm);
	out->append(buf, r);
}

void printDate (const google::protobuf::Message &m, std::string *out)
{
	const td::Date *d = static_cast<const td::Date*> (&m);
	char buf[32];
	sprintf(buf, "\"%04d-%02d-%02d\"", d->year(), d->month(), d->day());
	out->append(buf);
}

void printTime (const google::protobuf::Message &m, std::string *out)
{
	const td::TimeOfDay *d = static_cast<const td::TimeOfDay*> (&m);
	char buf[32];
	sprintf(buf, "\"%02d:%02d\"", d->hours(), d->minutes());
	out->append(buf);
}

int main (int argc, const char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s message_file.proto\n", (argc >= 1) ? argv[0] : "example1");
		return 1;
	}
	
	td::AddressBook root;
	
	const char *filename = argv[1];
	std::ifstream fHandle (filename, std::ios::in | std::ios::binary);
	if (!fHandle.is_open()) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		return 1;
	}
	
	if (!root.ParseFromIstream(&fHandle)) {
		fprintf(stderr, "Failed to parse file '%s'\n", filename);
		return 1;
	}
	
	std::string str;
	
	ProtobufJsonPrinter jsonPrint;
	
	jsonPrint.overrideType( google::protobuf::Timestamp::descriptor(), printTimestamp );
	jsonPrint.overrideType<td::Date> ( &printDate );
	jsonPrint.overrideType<td::TimeOfDay> ( &printTime );
	
	// These types are short in the Json representation, and are preferrable to read on one-line.
	jsonPrint.setTypeFlag( td::PhoneNumber::descriptor(), ProtobufJsonPrinter::TypeFlag::OnOneLine );
	
	jsonPrint.setTypeFlag( td::Address::descriptor(), ProtobufJsonPrinter::TypeFlag::OnOneLine );
	
	jsonPrint.toJson (root, &str);
	
	std::cout << str << "\n";
	
}

