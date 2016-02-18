#include <fstream>
#include <iomanip>
#include <sstream>
#include "WNXMLParser.h"
#include "WNQuery.h"

namespace LibWNXML {


WNQuery::WNQuery( const std::string& wnxmlfilename, ML::MultiLog& logger)
	: m_logger(logger)
{
	// open file
	std::ifstream inf( wnxmlfilename.c_str());
	if (!inf) {
		ML_THROW_EXC( "Could not open file: " << wnxmlfilename, WNQueryException);
	}

	// parse input file
	std::auto_ptr<WNXMLParser> psr = std::auto_ptr<WNXMLParser>( new WNXMLParser( "ISO-8859-2"));
	Synset syns;
	int lcnt = 0;
	while (!inf.eof()) {
		psr->parseXMLSynset( inf, syns, ++lcnt); // read next synset
		_save_synset( syns, lcnt);
	}
	// finish parsing (?call to xmlpp::SaxParser::finish_chunk_parsing() perhaps parses end of internal buffer?)
	psr->finishParsing();
	_save_synset( syns, lcnt);

	// invert relations
	invert_relations();

}


void WNQuery::_save_synset( Synset& syns, int lcnt)
{
	if (syns.empty())
		return;
	try {
		// check if id already exists, print warning if yes
		if (dat(syns.pos).find(syns.id) != dat(syns.pos).end()) {
			std::ostringstream os;
			os << "Warning W01: synset with this id (" << syns.id << ") already exists (input line " << lcnt << ")";
			m_logger.addLog( os.str(), 3);
			return;
		}
		// store synset
		dat(syns.pos)[ syns.id ] = syns;
		// index literals
		for (size_t i=0; i!=syns.synonyms.size(); i++)
			idx(syns.pos).insert( std::make_pair( syns.synonyms[i].literal, syns.id));
	}
	catch (const InvalidPOSException& e) {
		std::ostringstream os;
		os << "Warning W02: "<< e.msg() << " for synset in input line " << lcnt;
		m_logger.addLog( os.str(), 3);
	}
}


void WNQuery::invert_relations()
{
	// create inversion table
	std::map<std::string,std::string> inv;
	_invRelTable( inv);

	// nouns
	m_logger.addLog("Inverting relations for nouns...", 3);
	_inv_rel_pos( m_ndat, inv);
	// verbs
	m_logger.addLog("Inverting relations for verbs...", 3);
	_inv_rel_pos( m_vdat, inv);
	// adjectives
	m_logger.addLog("Inverting relations for adjectives...", 3);
	_inv_rel_pos( m_adat, inv);
	// adverbs
	m_logger.addLog("Inverting relations for adverbs...", 3);
	_inv_rel_pos( m_bdat, inv);
}

void WNQuery::_inv_rel_pos( tdat& dat, std::map<std::string,std::string>& inv)
{
	// for all synsets
	for (tdat::iterator it=dat.begin(); it!=dat.end(); it++) {
		// for all relations of synset
		for (size_t i=0; i!=it->second.ilrs.size(); i++) {
			// check if invertable
			std::map<std::string,std::string>::iterator invr = inv.find(it->second.ilrs[i].second);
			if (invr != inv.end()) {
				// check if target exists
				tdat::iterator tt = dat.find( it->second.ilrs[i].first);
				if (tt == dat.end()) {
					std::ostringstream os;
					os  << "Warning W03: synset " << it->second.ilrs[i].first << " is missing ('" << invr->first << "' target from synset " << it->first << ")";
					m_logger.addLog( os.str(), 3);
				}
				else {
					// check wether target is not the same as source
					if (tt->second.id == it->second.id) {
						std::ostringstream os;
						os  << "Warning W04: self-referencing relation '" << invr->second << "' for synset "  << it->second.id;
						m_logger.addLog( os.str(), 3);
					}
					else {
						// add inverse to target synset
						tt->second.ilrs.push_back( std::make_pair(it->first, invr->second));
						//std::ostringstream os;
						//os  << "Added inverted relation (target=" << it->first << ",type=" << invr->second << ") to synset " << tt->second.id;
						//m_logger.addLog( os.str(), 3);
					} // if target and source are not the same
				} // if target exists
			} // if invertible
		} // for all relations
	} // for all synsets
}


bool WNQuery::lookUpID( const std::string& id, const std::string& pos, Synset& syns) const
{
	syns.clear();
	tdat::const_iterator it = dat(pos).find( id);
	if (it == dat(pos).end())
		return false;
	else {
		syns = it->second;
		return true;
	}
}


bool WNQuery::lookUpLiteral( const std::string& literal, const std::string& pos, std::vector<Synset>& res) const
{
	res.clear();
	std::pair<tidx::const_iterator,tidx::const_iterator> ip = idx(pos).equal_range( literal);
	if (ip.first == ip.second)
		return false;
	for (tidx::const_iterator i=ip.first; i!=ip.second; i++) {
		Synset syns;
		if (lookUpID( i->second, pos, syns))
			res.push_back( syns);
	}
	return true;
}


bool WNQuery::lookUpLiteral( const std::string& literal, const std::string& pos, std::vector<std::string>& res) const
{
	res.clear();
	std::pair<tidx::const_iterator,tidx::const_iterator> ip = idx(pos).equal_range( literal);
	if (ip.first == ip.second)
		return false;
	for (tidx::const_iterator i=ip.first; i!=ip.second; i++)
		res.push_back( i->second);
	return true;
}


bool WNQuery::lookUpSense( const std::string& literal, const int sensenum, const std::string& pos, Synset& syns) const
{
	syns.clear();
	std::vector<Synset> res;
	if (!lookUpLiteral( literal, pos, res))
		return false;
	for (size_t i=0; i!=res.size(); i++) {
		for (size_t j=0; j!=res[i].synonyms.size(); j++) {
			if (res[i].synonyms[j].literal == literal && atoi(res[i].synonyms[j].sense.c_str()) == sensenum) {
				syns = res[i];
				return true;
			}
		}
	}
	return false;
}


void WNQuery::lookUpRelation( const std::string& id, const std::string& pos, const std::string& relation, std::vector<std::string>& targetIDs) const throw(InvalidPOSException)
{
	targetIDs.clear();
	// look up current synset
	tdat::const_iterator it = dat(pos).find( id);
	if (it == dat(pos).end()) // not found
		return;
	// get relation targets
	const std::vector< std::pair< std::string, std::string > >& ilrs = it->second.ilrs;
	for (size_t i=0; i!=ilrs.size(); i++)
		if (ilrs[i].second == relation)
			targetIDs.push_back( ilrs[i].first);
}


void WNQuery::traceRelation( const std::string& id, const std::string& pos, const std::string& relation, std::vector<std::string>& result) const
{
	result.clear();
	trace_rel_rec( id, pos, relation, result);
}


void WNQuery::trace_rel_rec( const std::string& id, const std::string& pos, const std::string& rel, std::vector<std::string>& res) const
{
	// get children
	std::vector<std::string> ids;
	lookUpRelation( id, pos, rel, ids);
	if (ids.empty()) // not found
		return;
	// save current synset
	res.push_back( id);
	// recurse on children
	for (size_t i=0; i!=ids.size(); i++) // for all relations of synset
		trace_rel_rec( ids[i], pos, rel, res); // recurse on target
}


void WNQuery::traceRelationOS( const std::string& id, const std::string& pos, const std::string& relation, std::ostream& os) const
{
	trace_rel_os_rec( id, pos, relation, os, 0);
}


void WNQuery::trace_rel_os_rec( const std::string& id, const std::string& pos, const std::string& rel, std::ostream& os, int lev) const
{
	// look up current synset
	tdat::const_iterator it = dat(pos).find( id);
	if (it == dat(pos).end()) // not found
		return;
	// print current synset
	for (int i=0; i<lev; i++) os << "  "; // indent
	os << it->second.id << "  {";
	for (size_t i=0; i!=it->second.synonyms.size(); i++) {
		os << it->second.synonyms[i].literal << ":" << it->second.synonyms[i].sense;
		if (i != it->second.synonyms.size()-1)
			os << ", ";
	}
	os << "}  (" << it->second.def << ")\n";
	// recurse on children
	lev++;
	for (size_t i=0; i!=it->second.ilrs.size(); i++) // for all relations of synset
		if (it->second.ilrs[i].second == rel) // if it is the right type
			trace_rel_os_rec( it->second.ilrs[i].first, pos, rel, os, lev); // recurse on target	
}


bool WNQuery::isIDConnectedWith( const std::string& id, const std::string& pos, const std::string& relation, const std::set<std::string>& target_ids,  std::string& foundTargetID) const
{
	foundTargetID = "";
	is_id_connected_with_rec( id, pos, relation, target_ids, foundTargetID);
	return ( foundTargetID != "");
}


void WNQuery::is_id_connected_with_rec( const std::string& id, const std::string& pos, const std::string& rel, const std::set<std::string>& targ_ids, std::string& foundTargetID) const
{
	// check if current synset is any of the searched ids
	if (targ_ids.count( id) != 0) { // found it
		foundTargetID = id;
		return; 
	}
	// look up / get children of current synset
	std::vector<std::string> children;
	lookUpRelation( id, pos, rel, children);
	if (children.empty())
		return;
	// recurse on children
	for (size_t i=0; i!=children.size(); i++) {
			if (foundTargetID != "") // check if not found already
				return;
			else 
				is_id_connected_with_rec( children[i], pos, rel, targ_ids, foundTargetID); // recurse on target
	}
}


bool WNQuery::isLiteralConnectedWith( const std::string& literal, const std::string& pos, const std::string& relation, const std::set<std::string>& targ_ids, std::string& foundID, std::string& foundTargetID) const
{
	foundID = "";
	foundTargetID = "";
	std::vector<std::string> ids;
	lookUpLiteral( literal, pos, ids);
	for (size_t i=0; i!=ids.size(); i++)
		if (isIDConnectedWith( ids[i], pos, relation, targ_ids, foundTargetID)) {
			foundID = ids[i];
			return true;
		}
	return false;
}


bool WNQuery::isLiteralCompatibleWithSynset( const std::string& literal, const std::string& pos, const std::string id, bool hyponyms) const
{
	// check if synset contains literal
	Synset syns;
	if (!lookUpID( id, pos, syns))
		return false;
	for (size_t i=0; i!=syns.synonyms.size(); i++)
		if (syns.synonyms[i].literal == literal)
			return true;
	// if allowed, recurse on hyponyms
	if (hyponyms) {
		for (size_t i=0; i!=syns.ilrs.size(); i++) {
			if (syns.ilrs[i].second == "hyponym") {
				if (isLiteralCompatibleWithSynset( literal, pos, syns.ilrs[i].first, true))
					return true;
			}
		}
	}
	return false;
}


bool WNQuery::areSynonyms(	const std::string& literal1, 
							const std::string& literal2,
							const std::string& pos,
							std::string& synsetid
						)  const
{
	synsetid = "";

	// get senses of input word1
	std::vector<std::string> senses1;
	lookUpLiteral( literal1, pos, senses1);

	// for each sense, check if it contains word2
	for (size_t i=0; i<senses1.size(); i++)
		if ( isLiteralCompatibleWithSynset( literal2, pos, senses1[i], false)) {
			synsetid = senses1[i];
			return true;
		}

	// no common synset
	return false;
}


void WNQuery::writeStats( std::ostream& os) const
{
	os << "PoS       \t#synsets\t#word senses\n";
	os << "Nouns     \t" << std::setw(8) << int(dat("n").size()) << "\t" << std::setw(11) << int(idx("n").size()) << std::endl;
	os << "Verbs     \t" << std::setw(8) << int(dat("v").size()) << "\t" << std::setw(11) << int(idx("v").size()) << std::endl;
	os << "Adjectives\t" << std::setw(8) << int(dat("a").size()) << "\t" << std::setw(11) << int(idx("a").size()) << std::endl;
	os << "Adverbs   \t" << std::setw(8) << int(dat("b").size()) << "\t" << std::setw(11) <<  int(idx("b").size()) << std::endl;
}


WNQuery::tdat&	WNQuery::dat( const std::string& pos)
{
	if (pos == "n")
		return m_ndat;
	else if (pos == "v")
		return m_vdat;
	else if (pos == "a")
		return m_adat;
	else if (pos == "b")
		return m_bdat;
	else {
		ML_THROW_EXC("Invalid POS '" << pos << "'", InvalidPOSException);
	}
}


const WNQuery::tdat& WNQuery::dat( const std::string& pos) const
{
	if (pos == "n")
		return m_ndat;
	else if (pos == "v")
		return m_vdat;
	else if (pos == "a")
		return m_adat;
	else if (pos == "b")
		return m_bdat;
	else {
		ML_THROW_EXC("Invalid POS '" << pos << "'", InvalidPOSException);
	}
}


WNQuery::tidx&	WNQuery::idx( const std::string& pos)
{
	if (pos == "n")
		return m_nidx;
	else if (pos == "v")
		return m_vidx;
	else if (pos == "a")
		return m_aidx;
	else if (pos == "b")
		return m_bidx;
	else {
		ML_THROW_EXC("Invalid POS '" << pos << "'", InvalidPOSException);
	}
}


const WNQuery::tidx&	WNQuery::idx( const std::string& pos) const
{
	if (pos == "n")
		return m_nidx;
	else if (pos == "v")
		return m_vidx;
	else if (pos == "a")
		return m_aidx;
	else if (pos == "b")
		return m_bidx;
	else {
		ML_THROW_EXC("Invalid POS '" << pos << "'", InvalidPOSException);
	}
}

} // namespace LibWNXML {

