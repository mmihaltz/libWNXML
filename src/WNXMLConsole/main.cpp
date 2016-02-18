/*

TODO:


*/


#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "..\CharConverter\CharConverter.h"
#include "../MLUtils/MultiLog.h"

#include "../LibWNXML/WNQuery.h"
#include "../SemFeatures/SemFeatures.h"


const ML::CharEncoding winenc = {ML::CharEncoding::ISO_8859_2, ML::CharEncoding::XT_CHREF_NORM};
const ML::CharEncoding dosenc = {ML::CharEncoding::MSDOS_852, ML::CharEncoding::XT_NONE};


/// Tokenize a string, delimited by either of characters given, into a vector of strings.
void split( const std::string& str, const std::string& tokchars, std::vector<std::string>& result)
{
//	ML::NewHandler newHandler;

	char *tok = NULL;
	char *buff = new char[str.size()+1];
    
	strcpy( buff, str.c_str());
	result.clear();

	tok = strtok( buff, tokchars.c_str());
	while (tok != NULL) {
		result.push_back( std::string( tok));
		tok = strtok( NULL, tokchars.c_str());
	}

	delete[] buff;
}


void write_synset( const LibWNXML::Synset& syns, std::ostream& outp)
{
	std::ostringstream os;
	
	os << syns.id << "  {";
	for (size_t i=0; i!=syns.synonyms.size(); i++) {
		os << syns.synonyms[i].literal << ":" << syns.synonyms[i].sense;
		if (i != syns.synonyms.size()-1)
			os << ", ";
	}
	os << "}  (" << syns.def << ")";
	
	std::auto_ptr<ML::CharConverter> cc = ML::CharConverter::create( winenc, dosenc);
	std::string str;
	cc->convert( os.str(), str);

	outp << str << std::endl;
}


void write_synset_id( const LibWNXML::WNQuery& wn, const std::string& id, const std::string& pos, std::ostream& outp)
{
	LibWNXML::Synset syns;
	wn.lookUpID( id, pos, syns);
	if (!syns.empty())
		write_synset( syns, outp);
}


void process_query( const LibWNXML::WNQuery& wn, ML_NPro2::SemFeatures* sf, const std::string& query, std::ostream& os)
{
	std::string str;
	std::auto_ptr<ML::CharConverter> cc = ML::CharConverter::create( dosenc, winenc);
	cc->convert( query, str);
	std::vector<std::string> t;
	split( str, " ", t);

	if (t[0] == ".h") { // .h
		os << "Available commands:\n";
		os << ".h                                               this help\n";
		os << ".q                                               quit\n";
		os << ".i   <id> <pos>                                   look up synset id in given POS (n,v,a,b)\n";
		os << ".l   <literal>                                    look up all synsets containing literal in all POS\n";
		os << ".l   <literal> <pos>                              look up all synsets containing literal in given POS\n";
		os << ".l   <literal> <sensenum> <pos>                   look up synset containing literal with given sense number in given POS\n";
		os << ".rl  <literal> <pos>                              list known relations of all senses of literal in POS\n";
		os << ".rl  <literal> <pos> <relation>                   look up relation (hypernym, hyponym) of all senses of literal with id and POS, list target ids\n";		
		os << ".ri  <id> <pos> <relation>                        look up relation of synset with id and POS, list target ids\n";
		os << ".ti  <id> <pos> <relation>                        trace relations of synset with id and POS\n";
		os << ".tl  <literal> <pos> <relation>                   trace relations of all senses of literal in POS\n";
		os << ".ci  <id> <pos> <relation> <id1> [<id2>...]       check if any of id1,id2,... is reachable from id by following relation\n";
		os << ".cl  <literal> <pos> <relation> <id1> [<id2>...]  check if any of id1,id2,... is reachable from any sense of literal by following relation\n";
		os << ".cli <literal> <pos> <id> [hyponyms]              check if synset contains literal, or if \"hyponyms\" is added, any of its hyponyms\n";
		os << ".slc <literal1> <literal2> <pos> <relation> [top] calculate Leacock-Chodorow similarity for all senses of literals in pos using relation\n";
		os << "                                                  if 'top' is added, an artificial root node is added to relation paths, making WN interconnected.\n";
		if (sf != NULL) {
			os << ".s  <feature>                                     look up semantic feature\n";
			os << ".sc <literal> <pos> <feature>                    check whether any sense of literal is compatible with semantic feature\n";
		}
		os << "\n";
	}

	else if (t[0] == ".i") { // .i
		if (t.size() != 3) {
			os << "Incorrect format for command .i\n\n";
			return;
		}
		LibWNXML::Synset syns;
		if (!wn.lookUpID( t[1], t[2], syns))
			os << "Synset not found\n\n";
		else {
			write_synset( syns, os);
			os << std::endl;
		}
	}

	else if (t[0] == ".l") { // .l
		if (t.size() != 2 && t.size() != 3 && t.size() != 4) {
			os << "Incorrect format for command .l\n\n";
			return;
		}
		if (t.size() == 2) { // .l <literal>
			std::vector<LibWNXML::Synset> res1, res;
			wn.lookUpLiteral(t[1], "n", res1);
			res.insert( res.end(), res1.begin(), res1.end());
			wn.lookUpLiteral(t[1], "v", res1);
			res.insert( res.end(), res1.begin(), res1.end());
			wn.lookUpLiteral(t[1], "a", res1);
			res.insert( res.end(), res1.begin(), res1.end());
			wn.lookUpLiteral(t[1], "b", res1);
			res.insert( res.end(), res1.begin(), res1.end());
			if (res.empty())
				os << "Literal not found\n\n";
			else {
				for (size_t i=0; i!=res.size(); i++)
					write_synset( res[i], os);
				os << std::endl;
			}
		}
		else if (t.size() == 3) { // .l <literal> <pos>
			std::vector<LibWNXML::Synset> res;
			if (!wn.lookUpLiteral(t[1], t[2], res))
				os << "Literal not found\n\n";
			else {
				for (size_t i=0; i!=res.size(); i++)
					write_synset( res[i], os);
				os << std::endl;
			}
		}
		else if (t.size() == 4) {  // .l <literal> <sensenum> <pos>
			LibWNXML::Synset syns;
			if (!wn.lookUpSense(t[1], atoi(t[2].c_str()), t[3], syns))
				os << "Word sense not found\n\n";
			else {
				write_synset( syns, os);
				os << std::endl;
			}
		}
	}

	else if (t[0] == ".rl") { // .rl
		if (t.size() != 4 && t.size() != 3) {
			os << "Incorrect format for command .rl\n\n";
			return;
		}
		if (t.size() == 3) { // .rl <literal> <pos>
			std::vector<LibWNXML::Synset> ss;
			wn.lookUpLiteral( t[1], t[2], ss);
			if (ss.empty())
				os << "Literal not found\n";
			else {
				std::set<std::string> rs;
				for (size_t j=0; j!=ss.size(); j++) {
					write_synset( ss[j], os);
					rs.clear();
					for (size_t i=0; i!=ss[j].ilrs.size(); i++)
						if (rs.count( ss[j].ilrs[i].second) == 0) {
							os << "  " << ss[j].ilrs[i].second << std::endl;
							rs.insert( ss[j].ilrs[i].second);
						}
					os << std::endl;
				}
			}
		}
		if (t.size() == 4) { // .rl <literal> <pos> <relation>
			std::vector<std::string> ss;
			wn.lookUpLiteral( t[1], t[2], ss);
			if (ss.empty())
				os << "Literal not found\n";
			else 
				for (size_t j=0; j!=ss.size(); j++) {
					write_synset_id( wn, ss[j], t[2], os);
					std::vector<std::string> ids;
					wn.lookUpRelation( ss[j], t[2], t[3], ids);
					if (!ids.empty())
						for (size_t i=0; i!=ids.size(); i++) {
							os << "  ";
							write_synset_id( wn, ids[i], t[2], os);
						}
					os << std::endl;
				}
		}
	}

	else if (t[0] == ".ri") { // .ri
		if (t.size() != 4) {
			os << "Incorrect format for command .ri\n\n";
			return;
		}
		std::vector<std::string> ids;
		wn.lookUpRelation( t[1], t[2], t[3], ids);
		if (ids.empty())
			os << "Synset not found or has no relations of the specified type\n";
		else
			for (size_t i=0; i!=ids.size(); i++)
				write_synset_id( wn, ids[i], t[2], os);
	}

	else if (t[0] == ".ti") { // .ti
		if (t.size() != 4) {
			os << "Incorrect format for command .ti\n\n";
			return;
		}
		std::ostringstream oss;
		wn.traceRelationOS( t[1], t[2], t[3], oss);
		if (oss.str().empty())
			os << "Synset not found\n\n";
		else {
			std::auto_ptr<ML::CharConverter> cc = ML::CharConverter::create( winenc, dosenc);
			std::string str;
			cc->convert( oss.str(), str);
			os << str << std::endl;
		}
	}

	else if (t[0] == ".tl") { // .tl
		if (t.size() != 4) {
			os << "Incorrect format for command .ti\n\n";
			return;
		}
		std::vector<LibWNXML::Synset> senses;
		if (!wn.lookUpLiteral( t[1], t[2], senses)) {
			os << "Literal not found\n\n";
			return;
		}
		for (size_t i=0; i!=senses.size(); i++) {
			std::ostringstream oss;
			wn.traceRelationOS( senses[i].id, t[2], t[3], oss);
			if (!oss.str().empty()) {
				std::auto_ptr<ML::CharConverter> cc = ML::CharConverter::create( winenc, dosenc);
				std::string str;
				cc->convert( oss.str(), str);
				os << str << std::endl;
			}
		}
	}

	else if (t[0] == ".ci") { // .ci
		if (t.size() < 5) {
			os << "Incorrect format for command .ci\n";
			return;
		}
		std::set<std::string> ids;
		for (size_t i=4; i!=t.size(); i++)
			ids.insert( t[i]);
		std::string foundtarg;
		if (wn.isIDConnectedWith( t[1], t[2], t[3], ids, foundtarg))
			os << "Connection found to " << foundtarg << std::endl;
		else
			os << "No connection found\n";
	}

	else if (t[0] == ".cl") { // .cl
		if (t.size() < 5) {
			os << "Incorrect format for command .cl\n";
			return;
		}
		std::set<std::string> ids;
		for (size_t i=4; i!=t.size(); i++)
			ids.insert( t[i]);
		std::string foundid;
		std::string foundtarg;
		if (wn.isLiteralConnectedWith( t[1], t[2], t[3], ids, foundid, foundtarg))
			os << "Connection found:\nSense of literal: " << foundid << "\nTarget id: " << foundtarg << std::endl;
		else
			os << "No connection found\n";
	}

	else if (t[0] == ".s") { // .s
		if (sf == NULL) {
			os << "Sorry, semantic features not loaded.\n";
			return;
		}
		if (t.size() != 2) {
			os << "Incorrect format for command .s\n";
			return;
		}
		std::set<std::string> ids;
		if (sf->lookUpFeature( t[1], ids)) {
			os << int(ids.size()) << " synset(s) found:\n";
			for (std::set<std::string>::iterator i=ids.begin(); i!=ids.end(); i++)
				os << *i << std::endl;
		}
		else {
			os << "Semantic feature not found\n";
		}
	}

	else if (t[0] == ".sc") { // .sc
		if (sf == NULL) {
			os << "Sorry, semantic features not loaded.\n";
			return;
		}
		if (t.size() != 4) {
			os << "Incorrect format for command .sc\n";
			return;
		}
		std::string foundid, foundtargid;
		if (sf->isLiteralCompatibleWithFeature( t[1], t[2], t[3], foundid, foundtargid)) {
			os << "Compatibility found:\n";		
			os << "Sense of literal: ";
			write_synset_id( wn, foundid, t[2], os);
			os << "Synset ID pertaining to feature: ";
			write_synset_id( wn, foundtargid, t[2], os);
		}
		else {
			os << "Compatibility not found\n";
		}
	}
	
	else if (t[0] == ".cli") { // .cli <literal> <pos> <id> [hyponyms]
		if ((t.size() != 4 && t.size() != 5) || (t.size() == 5 && t[4] !="hyponyms")) {
			os << "Incorrect format for command .cli\n";
			return;
		}
		bool hyps = (t.size() == 5 && t[4] == "hyponyms");
		if (wn.isLiteralCompatibleWithSynset( t[1], t[2], t[3], hyps))
			os << "Compatible\n";
		else
			os << "Not compatible\n";
	}
	
	else if (t[0] == ".slc") { // .slc <literal1> <literal2> <pos> <relation>
		if ((t.size() != 5 && t.size() != 6) || (t.size() == 6 && t[5] != "top")) {
			os << "Incorrect format for command .slc\n";
			return;
		}
		LibWNXML::WNQuery::tSims res;
		bool addtop = (t.size() == 6);
		wn.similarityLeacockChodorow( t[1], t[2], t[3], t[4], addtop, res);
		os << "Results:\n";
		for (LibWNXML::WNQuery::tSims::reverse_iterator it=res.rbegin(); it!=res.rend(); it++)
			os << "  " << it->first << "    " << it->second.first << "  " << it->second.second << "\n";
	}

	else {
		os << "Unknown command\n\n";
	}


}


int main( int argc, char *argv[])
{
	try {
		
		// check command line
		if (argc != 2 && argc != 3) {
			std::cerr << "Usage:\n  WNXMLConsole <WN_XML_file> [<semantic_features_XML_file>]\n";
			return 1;
		}

		// init WN
		std::cerr << "Reading XML...\n";
		std::auto_ptr<ML::MultiLog> logger( ML::MultiLog::create( std::cerr, 100));
		std::auto_ptr<LibWNXML::WNQuery> wn( new LibWNXML::WNQuery( argv[1], *logger));
		wn->writeStats( std::cerr);

		// init SemFeatures (if appl.)
		std::auto_ptr<ML_NPro2::SemFeatures> sf( NULL);
		if (argc == 3) {
			std::cerr << "Reading SemFeatures...\n";
			sf = std::auto_ptr<ML_NPro2::SemFeatures>( new ML_NPro2::SemFeatures( *wn));
			std::cerr << sf->readXML( argv[2]) << " pairs read\n";
		}

		// query loop
		std::cerr << "Type your query, or .h for help, .q to quit\n";
		std::string line;
		while (true) {	
			
			std::cerr << ">";
			std::getline( std::cin, line);

			if (line == ".q")
				break;
			else if (line != "") {
				try {
					process_query( *wn, sf.get(), line, std::cout);
				}
				catch (const LibWNXML::InvalidPOSException& e) {
					std::cerr << e.msg() << std::endl;
				}
			}

		} // while (true)

	} // try {
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown exception\n";
		return 1;
	}
	
	return 0;
}
