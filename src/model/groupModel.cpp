#include "groupModel.hpp"
#include "db.h"
// 群组管理类

// 创建群组
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into allgroup(groupname, groupdesc) \
     values('%s','%s')", group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}
// 加入群组
// user加入到组里面，角色是什么：role
void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into groupuser \
     values(%d, %d, '%s')", groupid,userid,role.c_str());
    MySQL mysql;
    if( mysql.connect()){
        mysql.update(sql);
    }
}
// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid)
{
    /*
        1.先根据userid在groupuser表中查询出该用户所属的群组信息
        2.根据群组信息，查询对应的群组用户信息
    */
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select a.id,a.name,a.desc from groupuser a \
     inner join user b on a.userid = b.id where b.id=%d", userid);
    vector<Group> groupVec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                // row[0] id
                // row[1] name
                // row[2] desc(组的功能)
                Group group(atoi(row[0]), row[1], row[2]);
                groupVec.push_back(group); 
            } 
            mysql_free_result(res);
        } 

    }

    // 查询用户群组信息,把群里面的所有用户都插入表中
    for(auto &group: groupVec)
    {
        sprintf(sql, "select a.id, a.name, a.state from user a inner \
             join groupuser b on a.id = b.userid where b.groupid=%d", group.getId());
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                GroupUser user(atoi(row[0]), row[1], row[2],row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;

}
// 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser \
         where groupid = %d and userid != %d", groupid, userid);
    vector<int> idVec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        } 
    }
    return idVec;
}
