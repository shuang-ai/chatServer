#ifndef GROUP_HPP
#define GROUP_HPP
#include <string>
#include <vector>
#include "groupuser.hpp"

using namespace std;


class Group {
public:
    Group(int id=-1, string name="", string desc="")
        :id(id), name(name), desc(desc){}
int getId(){return this->id;}
    string getName(){return this->name;}
    string getDesc(){return this->desc;}
    vector<GroupUser> getUsers(){return this->users;}
    void setId(int id){this->id = id;}
    void setName(string name){this->name = name;}
    void setDesc(string desc){this->desc = desc;}
    void setUsers(vector<GroupUser> &users){this->users = users;}
    
    private:
        int id;
        string name;
        string desc;
        vector<GroupUser> users;
};



#endif