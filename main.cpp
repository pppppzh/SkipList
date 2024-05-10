#include "skiplist.h"

int main()
{
    skiplist::SkipList<int, std::string> skipList(6);
	skipList.insert_element(1, "pppp"); 

	skipList.insert_element(3, "cccc"); 

	skipList.insert_element(7, "7777"); 

	skipList.insert_element(8, "qqqq"); 

	skipList.insert_element(9, "llll"); 
    skipList.display_list();
	skipList.insert_element(19, "zzzz"); 

	skipList.insert_element(19, "dddd"); 


    std::cout << "skipList size:" << skipList.size() << std::endl;


    // skipList.dump_file();

    // skipList.search_element(9);
    // skipList.search_element(18);


    // skipList.display_list();

    // skipList.delete_element(3);
    // skipList.delete_element(7);

    // std::cout << "skipList size:" << skipList.size() << std::endl;

    // skipList.display_list();

    return 0;
}