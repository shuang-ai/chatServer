#ifndef GROUPUSER_H
#define GROUPUSER_H
#include<vector>
#include<string>
// using namespace std;
class GroupUser
{
public:
    GroupUser(int id = -1, std::string name = "", std::string setState = "",
        std::string role = "normal")
        :id_(id), name_(name), setState_(setState),role_(role){}
    
        int getId(){return id_;}

        std::string getName(){return name_;}

        std::string getRole(){return role_;}

        std::string getState(){return setState_;}

    

        
    

private:
    // 在群里面的身份
    std::string role_;
    int id_;
    std::string name_;
    std::string setState_;
};

#endif