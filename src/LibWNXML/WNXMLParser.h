#ifndef __WNXMLPARSER_H__
#define __WNXMLPARSER_H__

#include <iostream>
#include <memory>
#include <string>
#include <libxml++/libxml++.h>
#include "../CharConverter/CharConverter.h"
#include "../mlutils/Exception.h"
#include "../mlutils/Throw.h"

#include "Synset.h"

//#define _LOGPARSE

namespace LibWNXML {


// This exception signals well-formedness or validity errors in the XML input, or other I/O errors
ML_EXCEPTION( WNXMLParserException);


class WNXMLParser : protected xmlpp::SaxParser
{
public:
    
	/// Constructor has 1 parameter:
	/// the character encoding you want in the parser's output
	/// default is UTF-8 (same as libxml's internal encoding)
	/// For valid encoding names, see CharConverter/EncodingNames.cpp
	WNXMLParser( std::string OutCharEnc = "UTF-8" );
	
	/// read xml input stream, parse one synset entry, then stop
	void	parseXMLSynset( std::istream& is, Synset& syns, int& linenum);
	
	/// Call this when finished parsing the input.
	void	finishParsing();

protected:
	// xmlpp::SaxParser overrides:

	//virtual void on_start_document();
	//virtual void on_end_document();
	//virtual void on_comment(const std::string& text);
	
	virtual void on_start_element(const std::string& name,
								  const AttributeList& properties);
	virtual void on_end_element(const std::string& name);
	virtual void on_characters(const std::string& characters);
		
	virtual void on_warning(const std::string& text)
		{	std::cerr << "libxml++ warning (input line " << m_lcnt << "): " << text << std::endl << m_line << std::endl; }
	virtual void on_error(const std::string& text)
		{	ML_THROW_EXC( "XML parser error (input line " << m_lcnt << "): " << text << std::endl << m_line, WNXMLParserException); }
	virtual void on_fatal_error(const std::string& text)
		{	ML_THROW_EXC( "XML parser fatal error (input line " << m_lcnt << "): " << text << std::endl << m_line << std::endl, WNXMLParserException); }

protected:

	// get current position in parse tree (1=root level, 2=1st level, ...)
	size_t getPos() 
		{ return m_ppath.size(); }

	// get name of ancestor at nth position in parse path 
	// (1=root node, getPos()-1=parent node, getPos()=current node), if it doesn't exist, returns empty string
	std::string getNParent(size_t n)
		{
			if (1 <= n && n <= m_ppath.size())
				return m_ppath[n-1];
			else
				return "";
		}
	
	// get the name of the parent node
	std::string getParent() 
		{ return getNParent( getPos()-1); }
	
	// for debugging
	void print_path( std::ostream& os)
		{
			for (size_t i=1; i <= getPos(); i++)
				os << "/" << getNParent( i);
		}
	
protected:

	int									m_lcnt;	// input line number
	std::string							m_line;	// last line of input
	std::vector<std::string>			m_ppath; // contains the XML path to the current node (names of the ancestors)
	int									m_done;	// -1: not started synset yet, 0: inside synset, 1: done with synset
	LibWNXML::Synset*						m_syns;	// points to the output struct
	std::auto_ptr<ML::CharConverter>	m_cconv; // char. encoding converter (from UTF-8 to enc. specified in constructor)
	bool								m_startroot; // was there a starting root tag?
	bool								m_endroot; // was there an end root tag?

};






}; // namespace LibWNXML

#endif // #ifndef __WNXMLPARSER_H__
