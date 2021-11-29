#include <bits/stdc++.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>
#include <termios.h>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <fstream>
#include <pwd.h>
#include <grp.h>

#define KEY_ESCAPE  0x001b
#define KEY_ENTER   0x000a
#define KEY_UP      0x0105
#define KEY_DOWN    0x0106
#define KEY_LEFT    0x0107
#define KEY_RIGHT   0x0108


#define cursor_up(x) printf("\033[%dA", (x))
#define cursor_down(x) printf("\033[%dB", (x))



using namespace std;


static struct termios term, oterm;
vector <pair<string,bool>> dflist;
char root[4000]; 
char cur_dir[5000]; 
stack <string> navl;
stack <string> navr;
int window=20;
struct termios raw,rawo;
vector <char> command_read;
vector <string> command_input;
int flag =0;
void command_mode(void);

static int get_ch(void)
{
    int c = 0;
    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    return c;
}

static int kb_hit(void)
{
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    if (c != -1) ungetc(c, stdin);
    return ((c != -1) ? 1 : 0);
}

static int kb_esc(void)
{
    int c;

    if (!kb_hit()) return KEY_ESCAPE;
    c = get_ch();
    if (c == '[') {
        switch (get_ch()) {
            case 'A':
                c = KEY_UP;
                break;
            case 'B':
                c = KEY_DOWN;
                break;
            case 'C':
                c = KEY_LEFT;
                break;
            case 'D':
                c = KEY_RIGHT;
                break;
            default:
                c = 0;
                break;
        }
    } else {
        c = 0;
    }
    if (c == 0) while (kb_hit()) get_ch();
    return c;
}

static int kb_get(void)
{
    int c;

    c = get_ch();
    return (c == KEY_ESCAPE) ? kb_esc() : c;
}

void clearscreen()
{
    printf("\033[H\033[J"); 
        
}
bool enableRawMode() {
    tcgetattr(0, &rawo); 
    tcgetattr(STDIN_FILENO, &raw);
     raw.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
     raw.c_iflag &= ~(BRKINT);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0)  
        return true;
    else
        return false;

}
void movecurser(int col, int row){
  cout<< "\033[" << col << ";" << row << "H";
}

bool isDir(const char* path)
{
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

void print_data(const char* pathf)
{
    if(strlen(pathf)<=12)
    {
        printf("%-14s ", pathf);
    }
    else
    {
        string pf =string(pathf);
        pf=pf.substr(0,10);
        printf("%-10s...  ",pf.c_str()); 
    }
    cout<<"\t";
    string cpath;
    cpath = string(cur_dir)+ "/" + string(pathf) ;
    char *path = new char[cpath.length() + 1];
    strcpy(path,cpath.c_str());
    struct stat buf;

    if(stat(path, &buf)==0)
    {
        if(S_ISDIR(buf.st_mode)) 
            cout<<"d";
        else
            cout<<"-";
        if(buf.st_mode & S_IRUSR)
            cout<<"r";
        else
            cout<<"-";
        if(buf.st_mode & S_IWUSR) 
            cout<<"w";
        else
            cout<<"-";
        if(buf.st_mode & S_IXUSR) 
            cout<<"x";
        else
            cout<<"-";
        if(buf.st_mode & S_IRGRP) 
            cout<<"r";
        else
            cout<<"-";
        if(buf.st_mode & S_IWGRP) 
            cout<<"w";
        else
            cout<<"-";
        if(buf.st_mode & S_IXGRP) 
            cout<<"x";
        else
            cout<<"-";
        if(buf.st_mode & S_IROTH)
            cout<<"r";
        else
            cout<<"-";
        if(buf.st_mode & S_IWOTH) 
            cout<<"w";
        else
            cout<<"-";
        if(buf.st_mode & S_IXOTH) 
            cout<<"x";
        else
            cout<<"-";

        cout<<"\t";
        struct passwd * un = getpwuid(buf.st_uid);
        printf("%10s ", un->pw_name);
        cout<<"\t";
        struct group * gn = getgrgid(buf.st_gid);
        printf("%10s ", gn->gr_name);
        cout<<"\t";
        long long int size=buf.st_size;
        printf("%10.2fKB", (size/1024.00));
        cout<<"\t";
        char *modtime = (ctime(&buf.st_mtime));
        modtime[strlen(modtime)] = '\0';
        printf("%20s", modtime);
    }
    else
    {
        perror("Error");
    }

}
int store_dir(const char* path)
{
    DIR *dir;
    struct dirent *ent;  
    dir = opendir (path);
    if ((dir == NULL)) {
        perror ("ERROR");
        return -1;
    }
    clearscreen();
    dflist.clear();
    movecurser(0,0);
    while ((ent = readdir (dir)) != NULL) {
        string s = string(path)+'/'+string(ent->d_name);
        if( isDir(s.c_str())) 
        {
            if(!(strcmp(ent->d_name,"..")==0 && strcmp(cur_dir,root)==0))
            {
                dflist.push_back(make_pair(ent->d_name,true) );
            }
        }
        else
        {
            dflist.push_back(make_pair(ent->d_name,false) );
        }
    }
    closedir (dir);
    
   return 0;
}
void display_dir(int st)
{
    clearscreen();
    int k=st+window;
    for(int i=st;i<dflist.size() && i<k;i++)
    {
        print_data(dflist[i].first.c_str());
    }
}






void enablescroll()
{
    movecurser(0,0);
    int cur=0;
    int scrptr=0;
    int c;

    while (1) {
        c = kb_get();
        if (c == KEY_ESCAPE ) {
            clearscreen();
            movecurser(0,0);
            cout<<"Closing Application\n";
            break;
        }else
        if (c == KEY_ENTER) {
            if(dflist[scrptr+cur].second==true)
            {
                if(!(strcmp(dflist[scrptr+cur].first.c_str(),"..")==0)) 
                {
                    if(!(strcmp(dflist[scrptr+cur].first.c_str(),".")==0))
                    {
                    string s = string(cur_dir)+'/'+dflist[scrptr+cur].first;
                    navr.push(s);
                    strcpy(cur_dir,s.c_str());
                    store_dir(cur_dir);
                    cur=0;
                    scrptr=0;
                    display_dir(0);
                    movecurser(0,0);
                    }
                   else{
                      movecurser(0,0);
                      cur=0;
                   }

                }
                else
                {
                    string s =string(cur_dir);
                    int i=s.length()-1;
                    for(;i>=0;i--)
                    {    if(s[i]=='/')
                        break;
                    }
                    s=s.substr(0,i);
                    strcpy(cur_dir,s.c_str());
                    navr.push(s);
                    cur=0;
                    scrptr=0;
                    store_dir(cur_dir);
                    display_dir(0);
                    movecurser(0,0);
                }
            }
            else
            {
                pid_t pid = fork();
                if (pid == 0) {
                 string p=string(cur_dir)+'/'+dflist[scrptr+cur].first;
                 execlp("/usr/bin/xdg-open", "xdg-open", p.c_str(), (char *)0);
                _exit(127);
                };
            }
        } else
        if (c == KEY_RIGHT) {
            if(navr.size()>1)
            {
                navl.push(navr.top());
                navr.pop();
                strcpy(cur_dir,navr.top().c_str());
                cur=0;
                scrptr=0;
                store_dir(cur_dir);
                display_dir(0);
                movecurser(0,0);
                
            }
        } else
        if (c == KEY_LEFT) {
            if(navl.size()>0)
            {
                navr.push(navl.top());
                navl.pop();
                strcpy(cur_dir,navr.top().c_str());
                cur=0;
                scrptr=0;
                store_dir(cur_dir);
                display_dir(0);
                movecurser(0,0);
            }
        }
         else
        if (c == KEY_UP) {
            if(cur!=0)
            {
                cur--;
                cursor_up(1);
            }   
        } else
        if (c == KEY_DOWN) {
            if(cur<min(window,(int)dflist.size())-1)
            {
                cur++;
                cursor_down(1);
            }
        } 
        else if(c=='k' || c=='K')
        {
            if(scrptr>0)
            {
                scrptr--;
                cur=0;
                display_dir(scrptr);
                movecurser(0,0);
            }
        }
        else if(c=='l' || c=='L')
        {
            if(scrptr<(int(dflist.size())-window-1))
            {
                scrptr++;
                cur=0;
                display_dir(scrptr);
                movecurser(0,0);
            }
        }
        else if(c==127)
        { 
            while (!navr.empty()) {
            navr.pop();
            }
            while (!navl.empty()) {
            navl.pop();
            }
            if(strcmp(cur_dir,root)!=0)
            {
            string s =string(cur_dir);
            int i=s.length()-1;
            for(;i>=0;i--)
            if(s[i]=='/')
            break;
            s=s.substr(0,i);
            strcpy(cur_dir,s.c_str());
            navr.push(s);
            cur=0;
            scrptr=0;
            store_dir(cur_dir);
            display_dir(0);
            movecurser(0,0);
            }
        }
        else if(c=='H' || c=='h')
        {
            strcpy(cur_dir,root);
            navr.push(string(cur_dir));
            cur=0;
            scrptr=0;
            store_dir(cur_dir);
            display_dir(0);
            movecurser(0,0);
        }
        else if(c==':')
        {
            command_mode();
            scrptr=0;
            cur=0;
            store_dir(cur_dir);
            display_dir(0);
            movecurser(0,0);
        }
        else if(c== 'q')
        {
            clearscreen();
            movecurser(0,0);
            cout<<"Closing Application\n";
            break;
        }
        
    }

}
void clearcmdwindow()
{
    int row=26;
    while(row<=40)
    {
        movecurser(row,0);   
        printf("%c[2K", 27);
        row++;
    }
    movecurser(26,0);
}
void parse_command()
{
    string str = "";
    long long int i =0;
    command_input.clear();
    while(i<command_read.size())
    {
        if (command_read[i] == '\n'  || command_read[i] == ' ' ) {
            if(int(str.length())>0)
            {
                command_input.push_back(str);
            }
            str="";
        }
        else{
            str+=command_read[i];
        }
        i++;
    }
}
string relative_to_absolute_conversion( string r_path)
{
    string a_path="";
    if(r_path[0]=='.' || r_path[0]=='~')
    {
        a_path = string(cur_dir) + r_path.substr(1,r_path.length());    
    }
    else if(r_path[0] =='/')
    {
        a_path = string(root) + r_path;
    }
    else
    {
        a_path= string(cur_dir)+ "/" + r_path;
    }

    return a_path;
}
void rename_file()
{
    if(command_input.size()==3)
    {
        string old_name =relative_to_absolute_conversion( command_input[1]);
        string new_name=relative_to_absolute_conversion(command_input[2]);
	    rename(old_name.c_str(), new_name.c_str());
    }
}

void copy_file(string source,string dest)
{
        ifstream input_file(source); 
        if (!input_file.is_open()) { 
                return;
            }
        ofstream output_file(dest);
        if (!output_file.is_open()) { 
            return;
        }
        output_file << input_file.rdbuf();
        struct stat st;
        stat(source.c_str(), &st);
        chmod(dest.c_str(), st.st_mode);
        chown(dest.c_str(), st.st_uid, st.st_gid);  
        input_file.close();
        output_file.close();
        return;
}
void copy_dir(string source,string dest)
{
     DIR *sdir;
    struct dirent *ent;  
    sdir = opendir (source.c_str());
    if ((sdir == NULL)) {
        perror ("ERROR");
        return ;
    }
    while ((ent = readdir (sdir)) != NULL) {
        string s = source+'/'+string(ent->d_name);
        string d = dest+'/'+string(ent->d_name);
        if( isDir(s.c_str())) 
        {
            if(!(strcmp(ent->d_name,"..")==0 || strcmp(ent->d_name,".")==0))
            {
                int p_temp=mkdir(d.c_str(),0755);
                if (p_temp==0)   
				{
					copy_dir(s,d);    
                }
                else
                {
					return ;  
                }
            }
        }
        else
        {
            copy_file(s,d);
        }
    }
    closedir (sdir);
    return ;

}


void create_file()
{
     if(command_input.size()>2)
            {
                char source[100];
                int i=1;
                string dir_path=relative_to_absolute_conversion(command_input[int(command_input.size())-1]);
                if(isDir(dir_path.c_str()))
                { 
                    while(i<int(command_input.size())-1){  
                    string dest_dir=dir_path+'/'+command_input[i];
                    strcpy(source, (dest_dir).c_str());
                    int fd1 = open(source, O_CREAT,0);
                    chmod(source,S_IRWXU | S_IRWXG | S_IROTH );
                    close(fd1);
                    i++;
                    }
                }
            }
}
void create_dir()
{   
    if(command_input.size()>2)
            {
                char source[1000];
                int i=1;
                string dir_path=relative_to_absolute_conversion(command_input[int(command_input.size())-1]);
                if(isDir(dir_path.c_str()))
                { 
                    while(i<int(command_input.size())-1){  
                    string dest_dir=dir_path+'/'+command_input[i];
                    strcpy(source, (dest_dir).c_str());
                    int stat = mkdir(source, S_IRWXU | S_IRWXG | S_IROTH );
                    i++;
                    }
                }
            }
}

void delete_file(string fpath)
{
    if(!isDir(fpath.c_str()))
    remove(fpath.c_str());
}
void delete_dir(string dpath)
{
    DIR *dir;
    struct dirent *ent;  
    dir = opendir (dpath.c_str());
    if ((dir == NULL)) {
        perror ("ERROR");
        return ;
    }
    while ((ent = readdir (dir)) != NULL) {
        string s = string(dpath)+'/'+string(ent->d_name);
        if( isDir(s.c_str())) 
        {
            if(!(strcmp(ent->d_name,"..")==0 || strcmp(ent->d_name,".")==0))
            {
                delete_dir(s);
                remove(s.c_str());
            }
        }
        else
        {
            remove(s.c_str());
        }
    }
    closedir (dir);
    return ;


}


void search(const char* path)
{
    DIR *dir;
    struct dirent *ent;  
    dir = opendir (path);
    if ((dir == NULL)) {
        perror ("ERROR");
        return ;
    }
    while ((ent = readdir (dir)) != NULL) {
        string s = string(path)+'/'+string(ent->d_name);
        if( isDir(s.c_str())) 
        {
            if(!(strcmp(ent->d_name,"..")==0 || strcmp(ent->d_name,".")==0))
            {
                if(string(ent->d_name)==command_input[1])
                {
                    cout<<"True ";
                    flag=1;
                }
                search(s.c_str());
            }
        }
        else
        {
            if(string(ent->d_name)==command_input[1])
            {
                cout<<"True ";
                flag=1;
            }
        }
    }
    closedir (dir);
    return ;
}
void goto_dir()
{
    if(command_input.size() == 2)
    {
        string dirpath =relative_to_absolute_conversion(command_input[1]);
        if(isDir(dirpath.c_str()))
        {
            strcpy(cur_dir,dirpath.c_str());
            navr.push(dirpath);
            store_dir(dirpath.c_str());
            display_dir(0);
            movecurser(25,0);
            cout<<"COMMAND MODE:";
        }
        else 
        {
            clearcmdwindow();
            cout<<"Wrong Path Given";
            getchar();
        }           
    }
}

void command_mode()
{
    movecurser(25,0);
    cout<<"COMMAND MODE:\n";
    int cursor_pos=26;
    int posy=0;
    fflush(0);
    char c[3];

    memset(c, 0, 3 * sizeof(c[0]));

    while (1) {
        if (read(STDIN_FILENO, c, 3) == 0)
                continue;
        if (c[0] == 27 ){ 
            if(c[1]==0 && c[2]==0)
            {
            clearcmdwindow();
            movecurser(25,0);   
            printf("%c[2K", 27);
            break;
            }
            if ( c[1] == '[' && (c[2] == 'A' || c[2] == 'B'|| c[2] == 'C' || c[2] == 'D')) {
            continue;
            }
        }else
        if (c[0] == 10) {
            command_read.push_back('\n');
            parse_command();
            if(command_input.size()>0)
            {
            if(command_input[0]=="rename")
            {
                rename_file();
                command_input.clear();
            }
            else if(command_input[0]=="search")
            {
                if(command_input.size()==2)
               {
                    clearcmdwindow();
                    flag=0;
                    search(cur_dir);
                    if(flag==0)
                        cout<<"FALSE";
                    cin.get();
                }
                command_input.clear();
            }
            else if(command_input[0]=="goto")
            {
                
                goto_dir();
                command_input.clear();
            }
            else if(command_input[0]=="delete_dir")
            {
                if(command_input.size()>1)
                {
                    int i=1;
                    while(i<int(command_input.size()))
                    {
                    string dpath=relative_to_absolute_conversion(command_input[i]); 
                        if(isDir(dpath.c_str()) )
                       { delete_dir(dpath);
                        remove(dpath.c_str());}
                    i++;
                    }
                }
                command_input.clear();
            }
            else if(command_input[0]=="delete_file")
            {
                if(command_input.size()>1)
                {
                    int i=1;
                    while(i<int(command_input.size()))
                    {
                    string fpath=relative_to_absolute_conversion(command_input[i]); 
                    delete_file(fpath);
                    i++;
                    }
                }
                command_input.clear();
            }
            else if(command_input[0]=="create_dir")
            {
                create_dir();
                command_input.clear();
            }
            else if(command_input[0]=="create_file")
            {
                create_file();
                command_input.clear();
            }
            else if(command_input[0]=="move")
            {
                if(command_input.size()>2)
                {
                int i=1;
                string dest=relative_to_absolute_conversion(command_input[int(command_input.size())-1]); 
                string dest1;
                if(isDir(dest.c_str()))
                {       
                while(i<int(command_input.size())-1)
                {
                    string source=relative_to_absolute_conversion(command_input[i]);
                    int len=int(source.length())-1;
                    for(;len>=0;len--)
                    {    if(source[len]=='/')
                        break;}
                    dest1=dest+source.substr(len,int(source.length())-len);
                     if(isDir(source.c_str()))
                        {
                            if(source==dest.substr(0,min(int(dest.length()),int(source.length()))))
                            {
                                clearcmdwindow();
                                cout<<"Cannot move a folder into itself";
                                getchar();
                            }
                            else{
                            int p_temp=mkdir(dest1.c_str(),0755);
                            if (p_temp==0)   
                            {
                                copy_dir(source,dest1);
                                delete_dir(source);
                                remove(source.c_str());
                            }
                            }
                        }
                        else{
                            copy_file(source,dest1);  
                            delete_file(source); 
                        }
                    i++;
                }
                }
            }
            command_input.clear();
            }
            else if(command_input[0]=="copy")
            {
                if(command_input.size()>2)
                {
                int i=1;
                string dest=relative_to_absolute_conversion(command_input[int(command_input.size())-1]); 
                string dest1;
                if(isDir(dest.c_str()))
                {
                        
                while(i<int(command_input.size())-1)
                {
                    string source=relative_to_absolute_conversion(command_input[i]);
                    int len=int(source.length())-1;
                    for(;len>=0;len--)
                    {    if(source[len]=='/')
                        break;}
                    dest1=dest+source.substr(len,int(source.length())-len);
                     if(isDir(source.c_str()))
                        {
                             if(source==dest.substr(0,min(int(dest.length()),int(source.length()))))
                            {
                                clearcmdwindow();
                                cout<<"Cannot copy a folder into itself";
                                getchar();
                            }
                            else
                            {
                            int p_temp=mkdir(dest1.c_str(),0755);
                            if (p_temp==0)   
                            {
                                copy_dir(source,dest1);
                            }
                            }
                        }
                        else{
                            copy_file(source,dest1);   
                        }
                    i++;
                }
                }
                }
                 command_input.clear();
            }
            else
            {
                if(!command_input.empty())
                command_input.clear();
            }
        }
            command_input.clear();
            clearcmdwindow();
            command_read.clear();
        } else
        if (c[0] == 127) {
            int i=0;
            if(command_read.size()>0)
            {
            command_read.pop_back();
            printf("%c[2K", 27);
            movecurser(cursor_pos,0);
            while(i<command_read.size())
            {
                cout<<command_read[i];
                i++;
            }
        }
        } else {
            command_read.push_back(c[0]);
            cout<<c[0];
        }
        fflush(0);
        memset(c, 0, 3 * sizeof(c[0]));
        }
    
return;

}



void normalmode(const char* path)
{
    clearscreen();
    store_dir(path);
    if(!enableRawMode())
    {
        cout<<"Error";
        return;
    } 
    display_dir(0);
    enablescroll();
}
int main ()
{
    clearscreen();
    char temp[256];
    string s;
    if(getcwd(temp,256)!=NULL)
    {
        s=temp;
    }
    strcpy(root,temp);
    strcpy(cur_dir,root);
    navr.push(s);
    normalmode(cur_dir);
    tcsetattr(0, TCSANOW, &rawo);
    return 1;
}
