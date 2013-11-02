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

static void usage(const char *execname)
{
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
        rename_file(argv[i], do_not_ask);
    
    return 0;
}
