#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <memory>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <iostream>
#include <gtk/gtk.h>
#include <memory>

#include "config_file.hpp"
#include "inifile.hpp"
#ifdef CONFIG_GNOME
# include "gconf_file.hpp"
#endif

using std::string;
using std::ostream_iterator;

typedef std::list<string> strlist;

std::ostream& operator<<(std::ostream& os, const strlist &sl)
{
	std::copy(sl.begin(), sl.end(), ostream_iterator<string>(os, "\n"));
	return os;
}

struct tmp_file {
	string fname;
	tmp_file(const string& fn) : fname(fn) {}
	~tmp_file() { 
		if (remove(fname.c_str())==-1) 
			std::cerr<<"can not remove: "<<fname<<". "<<strerror(errno)<<std::endl;
	}
};

#define TEST(type, def_val, some_val) do { \
		type type##_val=def_val; \
		cf->read_##type("/stardict-test", #type, type##_val); \
		if (type##_val!=def_val) { \
			std::cerr<<"type: "<<#type<<", default value was changed"<<std::endl;	\
			std::cerr<<"default: "<<def_val<<std::endl<<"have: "<<type##_val<<std::endl; \
			return false; \
		}												\
		cf->write_##type("/stardict-test", #type, some_val); \
		cf->read_##type("/stardict-test", #type, type##_val); \
		if (type##_val!=some_val) { \
			std::cerr<<"type: "<<#type<<", default value was NOT changed"<<std::endl;	\
			std::cerr<<"set to: "<<some_val<<std::endl<<"get: "<<type##_val<<std::endl; \
			return false; \
		}											\
	}	while (0)

static bool is_test_passed(config_file *cf)
{
	TEST(bool, true, false);
	TEST(int, 17, 18);
	TEST(string, "aaa", "bbb");
	strlist def_val, some_val;
	def_val.push_back("a");
	def_val.push_back("bbb");
	def_val.push_back("cdad");

	some_val.push_back("a");
	some_val.push_back("bbb");
	some_val.push_back("cdad");
	some_val.push_back("fdfad");

	TEST(strlist, def_val, some_val);

	return true;
}

#ifdef CONFIG_GNOME
static bool test_gconf()
{
	string tmp1=string(g_get_home_dir())+G_DIR_SEPARATOR_S+".gconf/stardict-test";
	string tmp2=string(g_get_home_dir())+G_DIR_SEPARATOR_S+".gconf/stardict-test/%gconf.xml";
	remove(tmp1.c_str());
	remove(tmp2.c_str());
	std::auto_ptr<config_file> cf;
	cf.reset(new gconf_file("/apps/stardict"));
	tmp_file tf=tmp1;
	tmp_file tf1=tmp2;
	bool res=is_test_passed(cf.get());
	cf.reset(0);
	system("gconftool-2 --shutdown");
	return res;
}
#endif

static bool test_inifile()
{
	std::auto_ptr<config_file> cf;
	string fname("/tmp/stardict.cfg");
	tmp_file tf(fname);
	cf.reset(new inifile(fname));
	bool res = is_test_passed(cf.get());
	cf.reset(0);
	return res;
}

int main(int argc, char *argv[])
{
	if (!test_inifile()) {
		std::cerr<<"ini file test failed"<<std::endl;
		return EXIT_FAILURE;
	}
		
#ifdef CONFIG_GNOME
	gtk_init(&argc, &argv);
	if (!test_gconf()) {
		std::cerr<<"Gconf test failed"<<std::endl;
		return EXIT_FAILURE;
	}
#endif
	
	return EXIT_SUCCESS;
}
