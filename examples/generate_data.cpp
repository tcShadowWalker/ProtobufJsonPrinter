#include "examples/example1.pb.h"
#include <google/protobuf/util/json_util.h>
#include <iostream>
#include <fstream>
#include <random>

namespace td = Example::Api;

std::mt19937 randGen (8888);

std::uint64_t genDigits (int ndigits) {
	return std::uniform_int_distribution<>( pow(10, ndigits - 1), pow(10, ndigits) - 1 )(randGen);
}
std::int64_t genBetween (std::int64_t min, std::int64_t max) {
	return std::uniform_int_distribution<>( min, max )(randGen);
}

typedef const std::vector<std::string> StringVec;

StringVec streets = { "Main Street", "Hayfield Gate", "Heywood Cedars", "Malham Cross"};
StringVec countries = {"Ruritania", "Timbuktu", "Waikikamukau", "Bongo Bongo Land"};
StringVec cities = {"Carjackastan", "Podunk", "Anytown"};

const std::string pickFromVec (StringVec &v)
{
	return v[ std::uniform_int_distribution<>(0, v.size() - 1)(randGen) ];
}

void generatePerson (td::Person *p, std::string name, std::string email, int nPhone, int nAddr)
{
	p->set_name(name);
	p->add_email(email);
	char buf[64];
	
	for(int i = 0; i < nPhone; ++i)
	{
		td::PhoneNumber *phone = p->add_phones();
		int r = snprintf(buf, sizeof(buf), "%.3lu-%.4lu-%.3lu", genDigits(3), genDigits(4), genDigits(3) );
		phone->set_number( std::string(buf, r) );
		phone->set_type ( (td::PhoneType) std::uniform_int_distribution<>(td::PhoneType_MIN, td::PhoneType_MAX)(randGen) );
	}
	
	for(int i = 0; i < nAddr; ++i)
	{
		td::Address *addr = p->add_addresses();
		addr->set_name( pickFromVec(streets) );
		addr->set_city( pickFromVec(cities) );
		addr->set_country( pickFromVec(countries) );
		addr->set_postalcode( genDigits(5) );
	}
	int secondsPerYear = 31536000;
	p->mutable_last_updated()->set_seconds ( 1483228800 + genBetween(0, secondsPerYear * 3 / ( 24 * 60)) * 24 * 60 );
	p->mutable_birthday()->set_year ( genBetween( 1970, 2000 ) );
	p->mutable_birthday()->set_month ( genBetween( 1, 12 ) );
	p->mutable_birthday()->set_day ( genBetween( 1, 29 ) );
	
	p->mutable_do_not_call_before()->set_hours( genBetween(8, 10) );
	p->mutable_do_not_call_before()->set_minutes( genBetween(0, 5) * 10 );
}

int main (int argc, const char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s message_file.proto\n", (argc >= 1) ? argv[0] : "generate_data");
		return 1;
	}
	
	const char *filename = argv[1];
	std::ofstream fHandle (filename, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!fHandle.is_open()) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		return 1;
	}
	
	td::AddressBook root;
	td::Person *person;
	
	person = root.add_people ();
	person->set_id ( root.people().size() );
	generatePerson(person, "John Smith", "jsmith@example.net", 2, 1);
	
	person = root.add_people ();
	person->set_id ( root.people().size() );
	generatePerson(person, "Jane Public", "jane@allyourbasearebelongto.us", 3, 2);
	
	person = root.add_people ();
	person->set_id ( root.people().size() );
	generatePerson(person, "Jock Tamson", "skynet555@example.org", 1, 1);
	
	person = root.add_people ();
	person->set_id ( root.people().size() );
	generatePerson(person, "Susie Macks", "yoga@macks.com", 2, 1);
	
	root.SerializeToOstream (&fHandle);
	
	{
		using namespace google::protobuf::util;
		JsonPrintOptions printOpt;
		printOpt.add_whitespace = true;
		
		std::string str;
		MessageToJsonString(root, &str, printOpt);
		std::cout << str;
	}
}

