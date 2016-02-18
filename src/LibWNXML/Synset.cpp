#include <cctype>

#include "Synset.h"

namespace LibWNXML {

std::ostream& operator << ( std::ostream& os, Synset ss)
{ return ss.writeXML( os); }

// replace: '&'->'&amp;', '<'->'&lt;', '>'->'&gt;', '''->'&apos;', '"'->'&quot;'
std::string EscPC( const std::string& str)
{
	std::string ret = str;
	for (unsigned int i=0; i<ret.size(); i++)
		switch (ret[i]) {
			case '&': {
					// check if it's not an entity reference!
					bool isentref = true;
					size_t j = ret.find_first_of( ';', i);
					if (j == ret.npos)
						isentref = false;
					else {
						for (size_t k=i+1; k<j && k<ret.size(); k++)
							if (!isalpha(ret[k]) && !isdigit( ret[k]) && ret[k]!='-' && ret[k]!='_' && ret[k]!='#') {
								isentref = false;
								break;
							}
					}
					if (!isentref) 
						ret.replace( i, 1, "&amp;");
				}
				break;
			case '<': ret.replace( i, 1, "&lt;"); break;
			case '>': ret.replace( i, 1, "&gt;"); break;
			case '\"': ret.replace( i, 1, "&quot;"); break;
			case '\'': ret.replace( i, 1, "&apos;"); break;
		}
	return ret;
} // XMLOStream::EscPC()


std::string tagstr( const std::string& tag, const std::string& str)
{
	return std::string( "<" + tag + ">" + EscPC(str) + "</" + tag + ">");
}

//std::string tagstr( const std::string& tag, int i)
//{
//	char buffer[64];
//	return std::string( "<" + tag + ">" + std::string( itoa( i, buffer, 10)) + "</" + tag + ">");
//}


void Synset::clear()
{
	id = pos = def = bcs = stamp = domain = nl = tnl = "";
	synonyms.clear();
	ilrs.clear();
	usages.clear();
	snotes.clear();
	sumolinks.clear();
	elrs.clear();
	ekszlinks.clear();
	vframelinks.clear();
}

std::ostream& Synset::writeXML( std::ostream& os)
{
	os << "<SYNSET>";
	
	os << tagstr( "ID", id);
	
	os << tagstr( "POS", pos);
	
	os << "<SYNONYM>";
	for (size_t i=0; i<synonyms.size(); i++) {
		os << "<LITERAL>";
		os << synonyms[i].literal;
		os << tagstr( "SENSE", synonyms[i].sense);
		if (synonyms[i].lnote != "")
			os << tagstr( "LNOTE", synonyms[i].lnote);
		if (synonyms[i].nucleus != "")
			os << tagstr( "NUCLEUS", synonyms[i].nucleus);
		os << "</LITERAL>";
	}
	os << "</SYNONYM>";

	for (size_t i=0; i<ilrs.size(); i++) {
		os << "<ILR>";
		os << ilrs[i].first;
		os << tagstr( "TYPE", ilrs[i].second);
		os << "</ILR>";
	}

	if (def != "")
		os << tagstr( "DEF", def);

	if (bcs != "")
		os << tagstr( "BCS", bcs);

	for (size_t i=0; i<usages.size(); i++)
		os << tagstr( "USAGE", usages[i]);

	for (size_t i=0; i<snotes.size(); i++)
		os << tagstr( "SNOTE", snotes[i]);

	if (stamp != "")
		os << tagstr( "STAMP", stamp);

	if (domain != "")
		os << tagstr( "DOMAIN", domain);

	for (size_t i=0; i<sumolinks.size(); i++) {
		os << "<SUMO>";
		os << sumolinks[i].first;
		os << tagstr( "TYPE", sumolinks[i].second);
		os << "</SUMO>";
	}

	if (nl != "")
		os << tagstr( "NL", nl);

	if (tnl != "")
		os << tagstr( "TNL", tnl);

	for (size_t i=0; i<elrs.size(); i++) {
		os << "<ELR>";
		os << elrs[i].first;
		os << tagstr( "TYPE", elrs[i].second);
		os << "</ELR>";
	}

	for (size_t i=0; i<ekszlinks.size(); i++) {
		os << "<EKSZ>";
		os << ekszlinks[i].first;
		os << tagstr( "TYPE", ekszlinks[i].second);
		os << "</EKSZ>";
	}

	for (size_t i=0; i<vframelinks.size(); i++) {
		os << "<VFRAME>";
		os << vframelinks[i].first;
		os << tagstr( "TYPE", vframelinks[i].second);
		os << "</VFRAME>";
	}

	os << "</SYNSET>";

	return os;
}

}; // namespace LibWNXML
