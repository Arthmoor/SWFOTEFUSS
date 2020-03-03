#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main()
{

// system("find */* | grep -v '\.' |xargs grep -e Site > sites");
// system("(find */* | grep -v '.' |xargs grep -e Site) > sites");
// system("grep -hr Site */*[^.clone] > sites");
 ifstream fin("sites");
 string s;
 string sites[500];
 int count = 1;
 int sitecount = 1;

 while( fin >> s )
 {
   if(count % 2 == 0)
   { 
//     cout << s << endl;
     sites[sitecount] = s;
     ++sitecount;
   }
   ++count;
 }

 int i=1;
 int z=1;
 int dupes=1;
 string duped[500];
 int numduped=1;
 bool isfirst=true;
 int listcount;
 while(i < sitecount)
 {
   while(z < sitecount)
   {
     if(sites[i] == sites[z] && i != z)
	++dupes;
     ++z;
   }
  if(dupes > 2)
  {
   for(listcount=1; listcount < sitecount; ++listcount)
   {
     if(duped[listcount] == sites[i])
     {
	isfirst=false;
	break;
     }
   }
   if(isfirst)
   {
   cout << sites[i] << " has " << dupes << " accounts" << endl;
   duped[numduped] = sites[i];
   ++numduped;
   }
  }
  dupes = 1;
  isfirst = true;
  z=1;
  ++i;
 }
  
} 
