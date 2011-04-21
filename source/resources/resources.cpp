
#include <iostream>
#include "resourcelist.h"
#include "resources.h"

namespace Resources
{
void Init()
{
	ResourceList::BuildList();

	cout << "built list of " << ResourceList::List().size() << " items" << endl;
	map< string, pair< u8*, u32 > >::const_iterator it = ResourceList::List().begin();
	while( it != ResourceList::List().end() )
	{
		cout << (*it).first << " " << (*it).second.second << endl;

		++it;
	}

}



}
