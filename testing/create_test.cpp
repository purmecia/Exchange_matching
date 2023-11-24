#include<iostream>
#include<string>
#include "tinyxml2.h"
#include "request.hpp"
using namespace tinyxml2;
using namespace std;

//function：create a xml file
int createXML(const char* xmlPath) {
        const char* declaration ="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        XMLDocument doc;
        doc.Parse(declaration);
        return doc.SaveFile(xmlPath);
}
class User {
public:
        User() {
                gender=0;
        };

        User(const string& userName, const string& password, int gender, const string& mobile, const string& email){
                this->userName=userName;
                this->password=password;
                this->gender=gender;
                this->mobile=mobile;
                this->email=email;
        };

        string userName;
        string password;
        int gender;
        string mobile;
        string email;
};

int insertXMLNode(const char* xmlPath,const User& user) {
        XMLDocument doc;
        int res=doc.LoadFile(xmlPath);
        if(res!=0)
        {
                cout<<"load xml file failed"<<endl;
                return res;
        }
        XMLElement* root=doc.RootElement();

        XMLElement* userNode = doc.NewElement("User");
        userNode->SetAttribute("Name", user.userName.c_str());
        userNode->SetAttribute("Password", user.password.c_str());
        root->InsertEndChild(userNode);

        XMLElement* gender = doc.NewElement("Gender");
        XMLText* genderText=doc.NewText(to_string(user.gender).c_str());
        gender->InsertEndChild(genderText);
        userNode->InsertEndChild(gender);

        XMLElement* mobile = doc.NewElement("Mobile");
        mobile->InsertEndChild(doc.NewText(user.mobile.c_str()));
        userNode->InsertEndChild(mobile);

        XMLElement* email = doc.NewElement("Email");
        email->InsertEndChild(doc.NewText(user.email.c_str()));
        userNode->InsertEndChild(email);

        return doc.SaveFile(xmlPath);
}

void xmlprint(const char* xmlPath) {
        XMLDocument doc;
        if(doc.LoadFile("./user.xml")!=0) {
                cout<<"load xml file failed"<<endl;
                return;
        }
        doc.Print();
}
int main(int argc,char* argv[]) {
        User user("lvlv","00001111",0,"13995648666","1586666@qq.com");
    createXML("./user.xml");
    insertXMLNode("./user.xml",user);
    xmlprint("./user.xml");

    XMLDocument doc;
    doc.LoadFile("./user.xml");
    XMLPrinter printer;
    doc.Print( &printer );
    cout<<"xml content length:"<<printer.CStrSize()<<endl;
    cout<<"xml content:"<<printer.CStr()<<endl;
    return 0;
}