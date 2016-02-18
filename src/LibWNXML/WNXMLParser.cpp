#include "../CharConverter/CharConverter.h"
#include "../CharConverter/EncodingNames.h"

#include "WNXMLParser.h"

namespace LibWNXML {


WNXMLParser::WNXMLParser( std::string OutCharEnc)
	: m_startroot( false)
	, m_endroot(false)
{
	// Set up character encoding converter
	// set output encoding
	ML::CharEncoding outenc;
	outenc.ext = ML::CharEncoding::XT_CHREF_NORM;
	outenc.enc = ML::EncodingNames::getEncoding( OutCharEnc.c_str());
	// check it
	if (outenc.enc == ML::CharEncoding::UNKNOWN) {
		ML_THROW_EXC( "Unknown character encoding '" << OutCharEnc << "' passed to WNXMLParser::WNXMLParser", WNXMLParserException);
	}
	// input encoding (UTF-8)
	ML::CharEncoding inenc;
	inenc.ext = ML::CharEncoding::XT_NONE;
	inenc.enc = ML::CharEncoding::UTF_8;
	// create enc. converter
	m_cconv = ML::CharConverter::create( inenc, outenc);
}


void WNXMLParser::parseXMLSynset( std::istream& is, Synset& syns, int& linenum)
{
	m_done = -1;
	m_syns = &syns;
	m_syns->clear();
	
	m_lcnt = linenum;
	while (!is.eof() && (m_done != 1)) {
		std::getline( is, m_line);
		m_lcnt++;
		linenum = m_lcnt;
		// VisDic XML format fault tolerance (no root tag):
		if (!m_startroot && m_line.find("<WNXML>") != m_line.npos)
			m_startroot = true;
		if (!m_startroot && m_line.find("<SYNSET>") != m_line.npos) {
			parse_chunk( "<WNXML>"); // fool parser
			m_startroot = true;
		}
		if (m_line.find("</WNXML>") != m_line.npos)
			m_endroot = true;
		// call the libxml++ SaxParser for this line
		try {
			parse_chunk( m_line);
		}
		catch (const WNXMLParserException& e) {
			ML_THROW_EXC( "Parser error on input line # " << linenum << ":\n" << m_line << std::endl << e.what(), WNXMLParserException);
		}
	}
	
	if (m_done == 0) // reached eof before end of segment
		ML_THROW_EXC( "Warning: end of file reached before </SYNSET>, possibly corrupt input", 
						WNXMLParserException);

}


void	WNXMLParser::finishParsing()
{ 
	// VisDic XML format fault tolerance (no root tag):
	if (!m_endroot)
		parse_chunk("</WNXML>"); // fool parser
	// this has to be called for whatever reason (see libxml++ API doc)
	finish_chunk_parsing(); 
}

void WNXMLParser::on_start_element(const std::string& name,
                                   const AttributeList& attrs)
{
#ifdef _LOGPARSE
	std::cout << m_lcnt << ": ";
	print_path( std::cout);
	std::cout << "/START: " << name << std::endl;
#endif

	m_ppath.push_back( name);

	if (m_done == 1) // already parsed a synset
		return;

	std::string parent   = getParent();
	std::string gparent  = getNParent( getPos()-2);
 
	if (name == "SYNSET")
		m_done = 0;

	else
	if (name == "LITERAL" && parent == "SYNONYM" && gparent == "SYNSET")
		m_syns->synonyms.push_back( Synset::Synonym( "", "", ""));

	else
	if (name == "ILR" && parent == "SYNSET")
		m_syns->ilrs.push_back( std::make_pair( "", ""));

	else
	if (name == "USAGE" && parent == "SYNSET")
		m_syns->usages.push_back( "");

	else
	if (name == "SNOTE" && parent == "SYNSET")
		m_syns->snotes.push_back( "");

	else
	if (name == "SUMO" && parent == "SYNSET")
		m_syns->sumolinks.push_back( std::make_pair( "", ""));

	else
	if (name == "EQ_NEAR_SYNONYM" && parent == "SYNSET")
		m_syns->elrs.push_back( std::make_pair( "", "eq_near_synonym"));		

	else
	if (name == "EQ_HYPERNYM" && parent == "SYNSET")
		m_syns->elrs.push_back( std::make_pair( "", "eq_has_hypernym"));

	else
	if (name == "EQ_HYPONYM" && parent == "SYNSET")
		m_syns->elrs.push_back( std::make_pair( "", "eq_has_hyponym"));

	else
	if (name == "ELR" && parent == "SYNSET")
		m_syns->elrs.push_back( std::make_pair( "", ""));

	else
	if (name == "EKSZ" && parent == "SYNSET")
		m_syns->ekszlinks.push_back( std::make_pair( "", ""));

	else
	if (name == "VFRAME" && parent == "SYNSET")
		m_syns->vframelinks.push_back( std::make_pair( "", ""));

}


void WNXMLParser::on_characters(const std::string& _text)
{
	// convert from UTF-8 to user-specified enc.
	std::string text;
	m_cconv->convert( _text, text);

#ifdef _LOGPARSE
	std::cout << m_lcnt << ": ";
	print_path( std::cout);	
	std::cout << "/#PCDATA: " << text << std::endl;
#endif

	if (m_done == 1 || m_done == -1)
		return;

	m_ppath.push_back( "#PCDATA");

	std::string parent   = getParent();
	std::string gparent  = getNParent( getPos()-2);
	std::string ggparent = getNParent( getPos()-3);

	if (parent == "ID" && gparent == "SYNSET") { // SYNSET/ID
		m_syns->id += text;
	}

	else
	if (parent == "POS" && gparent == "SYNSET") { // SYNSET/POS
		m_syns->pos += text;
	}

	else
	if (parent == "LITERAL" && gparent == "SYNONYM") { // SYNSET/SYNONYM/LITERAL
		if (m_syns->synonyms.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: synonyms empty at LITERAL tag", WNXMLParserException);
		}
		m_syns->synonyms.back().literal += text;
	}

	else
	if (parent == "SENSE" && gparent == "LITERAL" && ggparent == "SYNONYM") { // SYNSET/SYNONYM/LITERAL/SENSE
		if (m_syns->synonyms.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: synonyms empty at SENSE tag", WNXMLParserException);
		}
		m_syns->synonyms.back().sense += text;
	}

	else
	if (parent == "LNOTE" && gparent == "LITERAL" && ggparent == "SYNONYM") { // SYNSET/SYNONYM/LITERAL/LNOTE
		if (m_syns->synonyms.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: synonyms empty at LNOTE tag", WNXMLParserException);
		}
		m_syns->synonyms.back().lnote += text;
	}

	else
	if (parent == "NUCLEUS" && gparent == "LITERAL" && ggparent == "SYNONYM") { // SYNSET/SYNONYM/LITERAL/NUCLEUS
		if (m_syns->synonyms.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: synonyms empty at NUCLEUS tag", WNXMLParserException);
		}
		m_syns->synonyms.back().nucleus += text;
	}

	else
	if (parent == "ILR" && gparent == "SYNSET") { // SYNSET/ILR
		if (m_syns->ilrs.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: ilrs empty at ILR tag", WNXMLParserException);
		}
		m_syns->ilrs.back().first += text;
	}

	else
	if (parent == "TYPE" && gparent == "ILR") { // SYNSET/ILR/TYPE
		if (m_syns->ilrs.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: ilrs empty at ILR/TYPE tag", WNXMLParserException);
		}
		m_syns->ilrs.back().second += text;
	}

	else
	if (parent == "DEF" && gparent == "SYNSET") // SYNSET/DEF
		m_syns->def += text;

	else
	if (parent == "BCS" && gparent == "SYNSET") { // SYNSET/BCS
		m_syns->bcs += text;
	}

	else
	if (parent == "USAGE" && gparent == "SYNSET") { // SYNSET/USAGE 
		if (m_syns->usages.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: usages empty at USAGE tag", WNXMLParserException);
		}
		m_syns->usages.back() += text;
	}

	else
	if (parent == "SNOTE" && gparent == "SYNSET") { // SYNSET/SNOTE 
		if (m_syns->snotes.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: snotes empty at SNOTE tag", WNXMLParserException);
		}
		m_syns->snotes.back() += text;
	}

	else
	if (parent == "STAMP" && gparent == "SYNSET") { // SYNSET/STAMP 
		m_syns->stamp += text;
	}

	else
	if (parent == "DOMAIN" && gparent == "SYNSET") { // SYNSET/STAMP 
		m_syns->domain += text;
	}

	else
	if (parent == "SUMO" && gparent == "SYNSET") { // SYNSET/SUMO
		if (m_syns->sumolinks.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: sumolinks empty at SUMO tag", WNXMLParserException);
		}
		m_syns->sumolinks.back().first += text;
	}

	else
	if (parent == "TYPE" && gparent == "SUMO") { // SYNSET/SUMO/TYPE
		if (m_syns->sumolinks.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: sumolinks empty at SUMO/TYPE tag", WNXMLParserException);
		}
		m_syns->sumolinks.back().second += text;
	}

	else
	if (parent == "NL" && gparent == "SYNSET") { // SYNSET/NL 
		m_syns->nl += text;
	}

	else
	if (parent == "TNL" && gparent == "SYNSET") { // SYNSET/TNL 
		m_syns->tnl += text;
	}

	else
	if (parent == "EQ_NEAR_SYNONYM" && gparent == "SYNSET") { // SYNSET/EQ_NEAR_SYNONYM 
		if (m_syns->elrs.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: elrs empty at EQ_NEAR_SYNONYM tag", WNXMLParserException);
		}
		m_syns->elrs.back().first += text;
	}

	else
	if (parent == "EQ_HYPERNYM" && gparent == "SYNSET") { // SYNSET/EQ_HYPERNYM 
		if (m_syns->elrs.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: elrs empty at EQ_HYPERNYM tag", WNXMLParserException);
		}
		m_syns->elrs.back().first += text;
	}

	else
	if (parent == "EQ_HYPONYM" && gparent == "SYNSET") { // SYNSET/EQ_HYPONYM 
		if (m_syns->elrs.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: elrs empty at EQ_HYPONYM tag", WNXMLParserException);
		}		
		m_syns->elrs.back().first += text;
	}

	else
	if (parent == "ELR" && gparent == "SYNSET") { // SYNSET/ELR
		if (m_syns->elrs.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: elrs empty at ELR tag", WNXMLParserException);
		}
		m_syns->elrs.back().first += text;
	}

	else
		if (parent == "TYPE" && gparent == "ELR") { // SYNSET/ELR/TYPE
		if (m_syns->elrs.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: elrs empty at ELR/TYPE tag", WNXMLParserException);
		}
		m_syns->elrs.back().second += text;
	}

	else
	if (parent == "EKSZ" && gparent == "SYNSET") { // SYNSET/EKSZ
		if (m_syns->ekszlinks.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: ekszlinks empty at EKSZ tag", WNXMLParserException);
		}
		m_syns->ekszlinks.back().first += text;
	}

	else
		if (parent == "TYPE" && gparent == "EKSZ") { // SYNSET/EKSZ/TYPE
		if (m_syns->ekszlinks.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: ekszlinks empty at EKSZ/TYPE tag", WNXMLParserException);
		}
		m_syns->ekszlinks.back().second += text;
	}

	else
	if (parent == "VFRAME" && gparent == "SYNSET") { // SYNSET/VFRAME
		if (m_syns->vframelinks.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: vframelinks empty at VFRAME tag", WNXMLParserException);
		}
		m_syns->vframelinks.back().first += text;
	}

	else
		if (parent == "TYPE" && gparent == "VFRAME") { // SYNSET/VFRAME/TYPE
		if (m_syns->vframelinks.empty()) {
			ML_THROW_EXC( "WNXMLParser internal error: vframelinks empty at VFRAME/TYPE tag", WNXMLParserException);
		}
		m_syns->vframelinks.back().second += text;
	}

	m_ppath.pop_back();
}


void WNXMLParser::on_end_element(const std::string& name)
{
#ifdef _LOGPARSE
	std::cout << m_lcnt << ": ";
	print_path( std::cout);	
	std::cout << "/END: " << name << std::endl;
#endif

	if (m_done  == 1)
		return;

	if (name == "SYNSET") { // SYNSET
		if (m_done == -1) {
			ML_THROW_EXC( "This is impossible!\nThe parser should've caught this error: 'SYNSET' end tag without previous begin tag", WNXMLParserException);
		}
		m_done = 1;
	}

	m_ppath.pop_back();
}


} // namespace LibWNXML {