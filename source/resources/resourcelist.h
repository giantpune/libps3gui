#ifndef RESOURCELIST_H
#define RESOURCELIST_H


#include <map>
#include <string>
#include <utility>

#include <ppu-types.h>

// this stuff is for building a list of included resources at compile time and having them
// accessible by name at runtime

using namespace std;
namespace ResourceList
{
void BuildList();

const map< string, pair< u8*, u32 > > &List();

}

#endif // RESOURCELIST_H
