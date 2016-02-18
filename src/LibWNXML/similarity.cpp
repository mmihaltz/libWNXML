#include <iostream>
#include "WNQuery.h"

namespace LibWNXML {


void WNQuery::similarityLeacockChodorow(	const std::string& literal1, 
											const std::string& literal2,
											const std::string& pos,
											const std::string& relation,
											const bool addArtificialTop,
											tSims& results) const
{
	// clear output
	results.clear();
	
	// get senses of input words
	std::vector<std::string> senses1, senses2;
	lookUpLiteral( literal1, pos, senses1);
	lookUpLiteral( literal2, pos, senses2);
	if (senses1.empty() || senses2.empty()) // either of words not found
		return;

	// for each synset pair, calc similarity & put into results
	double s;
	for (size_t i=0; i!=senses1.size(); i++)
		for (size_t j=0; j!=senses2.size(); j++) {
			s = simLeaCho( senses1[i], senses2[j], pos, relation, addArtificialTop);
			results.insert( std::make_pair( s, std::make_pair( senses1[i], senses2[j])));
		}

}


double WNQuery::simLeaCho(	const std::string& id1, 
							const std::string& id2,
							const std::string& pos,
							const std::string& relation,
							const bool addArtificialTop) const
{
	// get nodes reachable from id1, id2 by relation + their distances (starting with id1/2 with dist. 1)
	std::vector< std::pair< std::string, int > > r1, r2;
	int d = 1;
	getReach( id1, pos, relation, r1, d, addArtificialTop);
	d = 1;
	getReach( id2, pos, relation, r2, d, addArtificialTop);

	// find common node (O(n*m))
	std::vector< std::pair< std::string, int > >::iterator ci_r1 = r1.end();
	std::vector< std::pair< std::string, int > >::iterator ci_r2 = r2.end();
	int path_length = 2*LeaCho_D;
	for (std::vector< std::pair< std::string, int > >::iterator i1=r1.begin(); i1!=r1.end(); i1++) {
		for (std::vector< std::pair< std::string, int > >::iterator i2=r2.begin(); i2!=r2.end(); i2++) {
			if (i1->first == i2->first) {
				if (i1->second + i2->second < path_length) {
					ci_r1 = i1;
					ci_r2 = i2;
					path_length = i1->second + i2->second;
				}
			}
		}
	}
	path_length = path_length - 1; // because the common node was counted twice

	//// debug: print info
	//std::cerr << "\nNodes in reach of " << id1 << ":\n";
	//for (size_t i=0; i!=r1.size(); i++)
	//	std::cerr << "  " << r1[i].first << "  " << r1[i].second << std::endl;
	//std::cerr << "Nodes in reach of " << id2 << ":\n";
	//for (size_t i=0; i!=r2.size(); i++)
	//	std::cerr << "  " << r2[i].first << "  " << r2[i].second << std::endl;
	//if (ci_r1 != r1.end() && ci_r2 != r2.end()) {
	//	std::cerr << "Common nodes with shortest path:\n";
	//	std::cerr << "  " << ci_r1->first << "  " << ci_r1->second << "\n";
	//	std::cerr << "  " << ci_r2->first << "  " << ci_r2->second << "\n";
	//	std::cerr << "Shortest path length:\n" << "  " << path_length << "\n";
	//	std::cerr << "Similarity score:" << "  " << - log10( double(path_length) / (double(2.0) * double(LeaCho_D)) ) << "\n\n";
	//
	//}
	//else {
	//	std::cerr << "No common node found.\n\n";
	//}

	// return similarity score
	if (ci_r1 != r1.end() && ci_r2 != r2.end()) // based on length of shortest connecting path
		return (double(-1.0) * log10( double(path_length) / (double(2.0) * double(LeaCho_D)) ));
	else // when no connecting path exists between synsets
		return LeaCho_noconnect;
}


void WNQuery::getReach(	const std::string& id,
						const std::string& pos,
						const std::string& rel,
						std::vector< std::pair< std::string, int > >& res,
						int dist,
						const bool addTop) const
{
	// look up current synset
	tdat::const_iterator it = dat(pos).find( id);
	if (it == dat(pos).end()) // not found
		return;
	// add current synset
	res.push_back( std::make_pair( id, dist));
	// recurse on children
	dist++;
	bool haschildren = false;
	for (size_t i=0; i!=it->second.ilrs.size(); i++) // for all relations of synset
		if (it->second.ilrs[i].second == rel) { // if it is the right type
			haschildren = true;
			getReach( it->second.ilrs[i].first, pos, rel, res, dist, addTop); // recurse on child
		}
	// if it has no "children" of this type (is terminal leaf or root level), add artificial "root" if requested
	if (!haschildren && addTop) 
		res.push_back( std::make_pair( "#TOP#", dist));
}




} // namespace LibWNXML {
