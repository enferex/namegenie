////////////////////////////////////////////////////////////////////////////////
// pdfrenamer - Rename PDFs to the first line in the pdf (presumably the title)
//
// Copyright (C) 2013, Matt Davis (enferex) <mattdavis9@gmail.com>
//
// pdfrenamer is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// pdfrenamer is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <unistd.h>
#include <libgen.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::ifstream;
using std::string;

// Version 0.1
static const int ver_major = 0;
static const int ver_minor = 1; 

static void usage(const char *execname)
{
    cout << "pdfrenamer v" << ver_major << "." << ver_minor << endl;
    cout << "Usage: " << execname << " [file.pdf ...] [-y]" << endl
         <<    "-y: Do not ask before renaming a file (good for batches)"
         << endl;
    exit(EXIT_SUCCESS);
}

// Remove whitespace from both ends of str
static string &strip(string &str)
{
    string::iterator ii;

    for (ii=str.begin(); ii!=str.end(); ++ii)
      if (!isspace(*ii) && isalnum(*ii))
        break;

    str.erase(str.begin(), ii);

    for (ii=str.begin(); ii!=str.end(); ++ii)
      if (*ii == '\n' || *ii == '\r')
        break;

    if (ii != str.end())
      str.erase(ii, str.end());

    return str;
}

// Given a pdf, open it, parse out the first line, remove spaces, and add a .pdf
// extension to this line.  That line will become (possibly) the new filename.
static string pdf_to_name(const char *fname)
{
    // Open the pdf
    poppler::document *doc = poppler::document::load_from_file(fname);
    if (!doc)
    {
        cerr << "Could not open document: '" << fname << "'" << endl;
        exit(EXIT_FAILURE);
    }

    // Get the first page's text
    poppler::page *pg = doc->create_page(0);
    if (!pg)
    {
        cerr << "Could not loacte the first page of the document" << endl;
        exit(EXIT_FAILURE);
    }

    // Convert first line to a string.
    string txt(pg->text().to_latin1());

    // Trim away whitespace and replace all spaces with '_'
    strip(txt);
    std::transform(txt.begin(), txt.end(), txt.begin(), ::tolower);
    std::replace(txt.begin(), txt.end(), ' ', '_');

    // Tack on a '.pdf' extension
    txt += ".pdf";
    cout << txt << endl;

    delete pg;
    delete doc;
    return txt;
}

// Given a pdf, optionally ask the user if they want to rename it, and do so.
static void rename_file(const char *fname, bool ask)
{
    char choice;
    string new_name = pdf_to_name(fname);

    // Error if the name is empty
    if (new_name.compare(".pdf") == 0)
    {
        cerr << "Could not find a good name for " 
             << fname << ", will not rename." << endl;
        return;
    }
    bool do_rename = false;

    // Get file path
    char *path_and_file = strdup(fname);
    string path(dirname(path_and_file));
    free(path_and_file);
    if (path.length() > 1)
      new_name = path + '/' + new_name;

    // Ask the user if they want to rename the file
    if (ask)
    {
        cout << "Rename " << fname << " to '" << new_name << "' [y/N]?";
        cin.get(choice);
        if (choice == 'y' || choice == 'Y')
            do_rename = true;
    }

    // If the new filename already exists, do not overwrite it
    ifstream fs(new_name.c_str());
    if (fs.good())
      do_rename = false;
          
    if (do_rename)
      if (rename(fname, (const char *)new_name.c_str()) != 0)
        cerr << "Could not rename file" << endl;
}

int main(int argc, char **argv)
{
    bool do_not_ask = false;

    if (argc == 1)
      usage(argv[0]);

    // Parse args
    for (int i=1; i<argc; ++i)
      if (strncmp(argv[i], "-y", strlen("-y")) == 0)
        do_not_ask = true;

    // Parse args again but just for filenames
    for (int i=1; i<argc; ++i)
      if (strncmp(argv[i], "-y", strlen("-y")) == 0)
        continue;
      else
        rename_file(argv[i], !do_not_ask);
    
    return 0;
}
