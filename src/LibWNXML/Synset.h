#ifndef __SYNSET_H__
#define __SYNSET_H__

#include <iosfwd>
#include <string>
#include <vector>

namespace LibWNXML {

class Synset
{
public:	// data members

	std::string id;
	std::string pos;

	struct Synonym
	{
		std::string literal;
		std::string	sense;
		std::string	lnote;
		std::string nucleus;

		Synonym( std::string l, std::string s, std::string o = "", std::string n = "") 
			: literal(l), sense(s), lnote(o), nucleus(n)
		{}
	};
	std::vector<Synonym> synonyms;

	typedef std::vector< std::pair< std::string, std::string> >	tPtrVect; /// Type for vector of "pointer", which are pairs whose 1st component it the link target (id), 2nd component is the link type
	
	tPtrVect					ilrs; // (target-id, rel-type) relation pointers

	std::string					def;
	std::string					bcs;
	std::vector<std::string>	usages;
	std::vector<std::string>	snotes;
	std::string					stamp;
	std::string					domain;
	tPtrVect					sumolinks; // (target-term, link-type) SUMO links
	std::string					nl;
	std::string					tnl;
	
	tPtrVect					elrs; // (target id, rel-type) pairs of external relation pointers

	tPtrVect					ekszlinks; // (sense-id, link-type) pairs of EKSz links

	tPtrVect					vframelinks; // (verb-frame-id, link-type) pairs of verb frame links

public: // member functions

	/// Constructor
	Synset()
	{ clear(); }

	/// clear/reset data members
	void clear();

	/// check if empty
	bool empty() const
	{ return id == ""; }

	/// Write VisDic XML representation of synset to stream
	std::ostream& writeXML( std::ostream& os);
	
}; // class Synset


/// Overloaded "<<" operator for writing VisDic XML representation of a Synset to an output stream
std::ostream& operator << ( std::ostream& os, Synset ss);


}; // namespace LibWNXML


#endif // #ifndef __SYNSET_H__
