#ifndef __WNQUERY_H__
#define __WNQUERY_H__

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif // #ifdef _MSC_VER

#include <iosfwd>
#include <math.h>
#include <map>
#include <set>
#include <string>
#include "../MLUtils/Exception.h"
#include "../MLUtils/Multilog.h"

#include "Synset.h"

namespace LibWNXML {

/// Exception for errors (during initialization, query, ...).
ML_EXCEPTION( WNQueryException);
/// To indicate invalid part-of-speech.
ML_EXCEPTION( InvalidPOSException);

/// Constants for the Leacock-Chodorow similarity measure
const int LeaCho_D = 20; ///< longest possible path from root to a node in WN
const double LeaCho_synonym = - log10( 1.0 / (2.0 * LeaCho_D)); ///< similarity score for synonyms (maximum possible similarity value), equals to approx. 1.60206 when D=20
const double LeaCho_noconnect = - 1.0; ///< similarity score for literals with no possible connecting path in WN (when similarity score is calculated with addArtificialTop = false option, see function header)

/// Class for querying WordNet, read from VisDic XML file
/// Character encoding of all results is ISO-8859-2 (Latin-2)
class WNQuery
{
public:

	/// Constructor. Create the object: read XML file, create internal indices, invert invertable relations etc.
	/// @param wnxmlfilename file name of VisDic XML file holding the WordNet you want to query
	/// @param logger ML::MultiLog for writing warnings (e.g. invalid POS etc.) to while loading. The default value creates a logger to stderr.
	/// The following warnings may be produced:
	/// Warning W01: synset with id already exists 
	/// Warning W02: invalid PoS for synset (NOTE: these synsets are omitted)
	/// Warning W03: synset is missing (the target synset, when checking when inverting relations)
	/// Warning W04: self-referencing relation in synset
	/// @exception WNQueryException thrown if input parsing error occurs
	WNQuery( const std::string& wnxmlfilename, ML::MultiLog& logger )	throw(WNQueryException);

	/// Get synset with given id.
	/// @param id synset id to look up
	/// @param pos POS of synset
	/// @param syns the result synset. Empty if no synset found.
	/// @return true if synset was found, false otherwise
	/// @exception InvalidPOSException for invalid POS
	bool lookUpID( const std::string& id, const std::string& pos, Synset& syns)	const throw(InvalidPOSException);

	/// Get synsets containing given literal in given POS.
	/// @param literal to look up (all senses)
	/// @param pos POS of literal
	/// @param results contains the Synsets with the id, empty if not found
	/// @return true if liteal was found, false otherwise
	/// @exception InvalidPOSException for invalid POS
	bool lookUpLiteral( const std::string& literal, const std::string& pos, std::vector<Synset>& results) const throw(InvalidPOSException);

	/// Get ids of synsets containing given literal in given POS.
	bool lookUpLiteral( const std::string& literal, const std::string& pos, std::vector<std::string>& results) const throw(InvalidPOSException);

	/// Get synset containing word sense (literal with given sense number) in given POS.
	/// @param literal to look up
	/// @param sensenum sense number of literal
	/// @param pos POS of literal
	/// @param syns the synset containing the word sense, empty if not found
	/// @return true if word sense was found, false otherwise
	/// @exception InvalidPOSException for invalid POS
	bool lookUpSense( const std::string& literal, const int sensenum, const std::string& pos, Synset& syns) const throw(InvalidPOSException);

	/// Get IDs of synsets reachable from synset by relation
	/// @param id synset id to look relation from
	/// @param pos POS of starting synset
	/// @param relation name of relation to look up
	/// @param targetIDs ids of synsets found go here (empty if starting synset was not found, or if no relation was found from it)
	/// @exception InvalidPOSException for invalid POS
	void lookUpRelation( const std::string& id, const std::string& pos, const std::string& relation, std::vector<std::string>& targetIDs) const throw(InvalidPOSException);
	
	/// Do a recursive trace from the given synset along the given relation.
	/// @param id id of synset to start from
	/// @param pos POS of search
	/// @param relation name of relation to trace
	/// @param result holds the ids of synsets found on the trace. It's empty if starting synset is not found, otherwise always holds at least the starting synset (so if starting synset has no relations of the searched type, result will only hold that synset)
	/// @exception InvalidPOSException for invalid POS
	void traceRelation( const std::string& id, const std::string& pos, const std::string& relation, std::vector<std::string>& result) const throw(InvalidPOSException);

	/// Like TraceRelation, but output goes to output stream with pretty formatting.
	void traceRelationOS( const std::string& id, const std::string& pos, const std::string& relation, std::ostream& os) const throw(InvalidPOSException);

	/// Check if synset is connected with any of the given synsets on paths defined by relation starting from synset.
	bool isIDConnectedWith( const std::string& id, const std::string& pos, const std::string& relation, const std::set<std::string>& targetIDs, std::string& foundTargetID) const throw(InvalidPOSException);

	/// Check if any sense of literal in POS is connected with any of the specified synsets on paths defined by relation starting from that sense.
	bool isLiteralConnectedWith( const std::string& literal, const std::string& pos, const std::string& relation, const std::set<std::string>& targetIDs, std::string& foundID, std::string& foundTargetID) const throw(InvalidPOSException);

	/// Check if literal is in synset, or, if hyponyms is true, is in one of synset's hyponyms (recursive)
	bool isLiteralCompatibleWithSynset( const std::string& literal, const std::string& pos, const std::string id, bool hyponyms) const  throw(InvalidPOSException);

	/// Calculate Leacock-Chodorow similarity between two words using WN.
	/// @param literal1, literal2 the two input words
	/// @param pos PoS of the words (n,v,a,b)
	/// @param relation the name of the relation to use for finding connecting paths
	/// @param addArtificialTop if true, add an artificial top (root) node to ends of relation paths
	/// so that the whole WN graph will be interconnected. If false, there can literals with zero connections (empty results map, see below).
	/// @param results the results: for every pair of synset ids of all the senses of the input words, the similarity score,
	/// or empty if either of the 2 words was not found in WN. For a score, first element of the pair of strings is the id of a sense of literal1,
	/// second element is the id of a sense of literal 2. The map is cleared by the function first.
	/// @exception InvalidPOSException for invalid POS	
	/// Description of method:
	/// We first look up all the senses of the 2 input words in the given PoS.
	/// Then, for every possible pair (s1,s2) we calculate the formula:
	/// sim(s1,s2) = - log ( length_of_shortest_path( s1, s2, relation) / 2 * D)
	/// where D is a constant (should be at least as much as the longest path in WN using relation, see .cpp file for values)
	/// length_of_shortest_path is the number of nodes found in the path connecting s1 and s2 with relation, so
	/// if s1 = s2, length_of_shortest_path = 1, 
	/// if s1 is hypernym of s2 (or vice versa), length_of_shortest_path = 2,
	/// if s1 and s2 are sister nodes (have a common hypernym), length_of_shortest_path = 3, etc.
	/// Note, when addArtificialTop is true, the formula returns a sim. score of 1.12494 (for path length 3) 
	/// for invalid relation types (since since the path always contains the starting node, plus the artificial root node).
	typedef std::map< double, std::pair<std::string,std::string> > tSims;
	void similarityLeacockChodorow(	const std::string& literal1, 
									const std::string& literal2,
									const std::string& pos,
									const std::string& relation,
									const bool addArtificialTop,
									tSims& results) const throw(InvalidPOSException);

	/// Determine if two literals are synonyms in a PoS, also return id of a synset that contains both.
	/// @param literal1 first word to be checked
	/// @param literal2 second word to be checked
	/// @param pos PoS of the words (n,v,a,b)
	/// @param synsetid if there is a synset in 'pos' that contains both literal1 and literal2, its id is returned here.
	/// Note, there may be more synsets containing these two words.
	/// @return true if the two words are synonyms, false otherwise.
	/// @exception InvalidPOSException for invalid POS
	bool areSynonyms(	const std::string& literal1, 
						const std::string& literal2,
						const std::string& pos,
						std::string& synsetid
						)  const throw(InvalidPOSException);

	/// Write statistics about number of synsets, word senses for each POS.
	/// @param os the output stream to write to
	void writeStats( std::ostream& os) const;

public:

	/// The following functions give access to the internal representation of
	/// all the content read from the XML file.
	/// Use at your own risk.

	/// synset ids to synsets
	typedef std::map<std::string, LibWNXML::Synset> tdat;

	/// literals to synset ids
	typedef std::multimap<std::string, std::string> tidx;

	/// Get the appropriate synset-id-to-synset-map for the given POS.
	/// @param pos part-of-speech: n|v|a|b
	/// @exception WNQueryException if invalid POS
	tdat&			dat( const std::string& pos)		throw(WNQueryException);
	const	tdat&	dat( const std::string& pos) const	throw(WNQueryException);

	/// Get the appropriate literal-to-synset-ids-multimap for the given POS.
	/// @param pos part-of-speech: n|v|a|b
	/// @exception WNQueryException if invalid POS
	tidx&			idx( const std::string& pos)		throw(WNQueryException);
	const	tidx&	idx( const std::string& pos) const	throw(WNQueryException);

private:

	void _save_synset( Synset& syns, int lcnt);
	
	void trace_rel_rec( const std::string& id, const std::string& pos, const std::string& relation, std::vector<std::string>& result) const;

	void trace_rel_os_rec( const std::string& id, const std::string& pos, const std::string& relation, std::ostream& os, int level) const;

	void is_id_connected_with_rec( const std::string& id, const std::string& pos, const std::string& relation, const std::set<std::string>& ancestor_ids, std::string& foundAncestorID) const throw(InvalidPOSException);

	/// Create the inverse pairs of all reflexive relations in all POS.
	/// Ie. if rel points from s1 to s2, mark inv(rel) from s2 to s1.
	/// see body of _invRelTable().
	void invert_relations();
	void _inv_rel_pos( tdat& pdat, std::map<std::string,std::string>& invtbl);
	void _invRelTable( std::map<std::string,std::string>& inv)
	{
		inv.clear();
		inv["hypernym"]					= "hyponym";
		inv["holo_member"]				= "mero_member";
		inv["holo_part"]				= "mero_part";
		inv["holo_portion"]				= "mero_portion";
		inv["region_domain"]			= "region_member";
		inv["usage_domain"]				= "usage_member";
		inv["category_domain"]			= "category_member";
		inv["near_antonym"]				= "near_antonym";
		inv["middle"]					= "middle";
		inv["verb_group"]				= "verb_group";
		inv["similar_to"]				= "similar_to";
		inv["also_see"]					= "also_see";
		inv["be_in_state"]				= "be_in_state";
		inv["eng_derivative"]			= "eng_derivative";
		inv["is_consequent_state_of"]	= "has_consequent_state";
		inv["is_preparatory_phase_of"]	= "has_preparatory_phase";
		inv["is_telos_of"]				= "has_telos";
		inv["subevent"]					= "has_subevent";
		inv["causes"]					= "caused_by";
	}

	double WNQuery::simLeaCho(	const std::string& id1, 
								const std::string& id2,
								const std::string& pos,
								const std::string& relation,
								const bool addArtificialTop) const;

	void getReach(	const std::string& id,
					const std::string& pos,
					const std::string& rel,
					std::vector< std::pair< std::string, int > >& res,
					int dist,
					const bool addArtificialTop) const;

private:

	ML::MultiLog&	m_logger;

	tdat		m_ndat; ///< nouns
	tdat		m_vdat;
	tdat		m_adat;
	tdat		m_bdat;

	tidx		m_nidx; ///< nouns
	tidx		m_vidx;
	tidx		m_aidx;
	tidx		m_bidx;

};


} // namespace LibWNXML {

#endif // #ifndef __WNQUERY_H__
