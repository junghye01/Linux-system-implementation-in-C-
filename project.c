#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include <grp.h> 
#include<string.h>
#include<errno.h>
#include <sys/stat.h> 
#include<pwd.h>
#include<pthread.h>
#define PERM 0644

typedef struct two {
    char* name;
    char num;
}two;
void print_error(char* this, char* filename1, char* filename2)
{
    fprintf(stderr, "%s cannot move %s to %s\n%s\n", this, filename1, filename2, strerror(errno));
    exit(EXIT_FAILURE);
}

void print_usage(char* this) {
    fprintf(stderr, "SYNTAX ERROR:\nUsage %s[old_filename] [newfilename]", this);
    exit(EXIT_FAILURE);
}

void append(char* dst, char c) {
    char* p = dst;
    while (*p != '\0') p++; // 문자열 끝 탐색
    *p = c;
    *(p + 1) = '\0';
}

void* makedir(void* fname)
{
    int s;
    char* filename = (char*)fname;
    s = mkdir(filename, 0755);
    if (s < 0)
    {
        perror("directory create error ");
    }

}
void RM(char* filename) {
    unlink(filename);
}

void mrm(int argc,char* filenames[])
{
  int num=argc-2;
  int thr_id;
  pthread_t p_thread[num];
  for (int i = 0; i < num; i++)
        {
            thr_id = pthread_create(&p_thread[i], NULL, RM, (void*)filenames[i+2]);
            if (thr_id < 0)
            {
                perror("thread create error : ");
                exit(0);
            }
        }
        for (int i = 0; i < num; i++)
    {
        pthread_join(p_thread[i], NULL);//스레드가 종료될떄까지 기다려줌
    }
}

void Mmkdir(int argc, char* argv[])//여러개의 디엑토리를 생성하는 함수.
{
    int num = argc - 2;
    pthread_t p_thread[num];
    int thr_id;
    if (strcmp(argv[1], "mkdir") == 0) {
        for (int i = 0; i < num; i++)
        {
            thr_id = pthread_create(&p_thread[i], NULL, makedir, (void*)argv[i + 2]);//스헤드를 생성하여 makedir함수를 실행
            if (thr_id < 0)
            {
                perror("thread create error : ");
                exit(0);
            }
        }
    }
    for (int i = 0; i < num; i++)
    {
        pthread_join(p_thread[i], NULL);//스레드가 종료될떄까지 기다려줌
    }
}


void Omkdir(char* op, char* filename, char* filemode)//하나의 디렉토리를 생성하는 옵션
{
    int result = 0;
    int mode = (filemode[0] - 48) * 64 + (filemode[1] - 48) * 8 + (filemode[2] - 48);
    if (strcmp(op, "-p") == 0)
    {
        int t = strlen(filename);
        char* path;
        for (int i = 0; i <= t; i++)
        {
            path[i] = filename[i];
            if (path[i] == '/' || path[i] == '\0') {//파일과 디렉토리를 / 기준으로 나눠줌
                path[i] = '\0';//디렉토리 이름이 완료됐음을 표시해줌
                result = mkdir(path, mode);
                path[i] = '/';
                if(result<0)
                  printf("파일 생성에 실패했습니다");
            }
        }
    }

    else {
        result = mkdir(filename, mode);
        if(result<0)
          printf("파일 생성에 실패했습니다");
    }
}

void Cat(int argc, char* op, char* filename) {//cat함수
    int fdin;
    int cnt = 1;
    char buf[BUFSIZ];
    if (argc == 3) {//파일의 정보를 읽오 오는 함수. //cat file1
        char* pLine;
        FILE* fp = fopen(filename, "r");
        pLine = fgets(buf, BUFSIZ, fp);
        while (!feof(fp)) {//파일이 끝날떄까지 라인을 불러와서 저장하기
            pLine = fgets(buf, BUFSIZ, fp);
            if (pLine != NULL) {
                printf("%s", pLine);//라인을 불러올때마다 출력
            }
        }
        fclose(fp);
    }
    
    if (argc == 4) {//파일을 생성하는 옵션
        if (strcmp(op, ">") == 0) //cat > newfile
        {
            FILE* fp = fopen(filename, "w");
            char c;
            while ((c = getchar()) != EOF) {//ctrl+D로 저장하고 종료
                fputc(c, fp);
            }
            fclose(fp);
        }
        if (strcmp(op, "-n") == 0)//파일에 줄을 붙혀서 출력해주는 옵션
        {
            char* pLine;
            FILE* fp = fopen(filename, "r");
            pLine = fgets(buf, BUFSIZ, fp);
            while (!feof(fp)) {
                pLine = fgets(buf, BUFSIZ, fp);
                if (pLine != NULL) {
                    printf("%d", cnt++);
                    printf("%s", pLine);
                }
            }
            fclose(fp);
        }
    }
}

void myls(int argc, char* op)
{
    int opnum = strlen(op);
    char* cwd = (char*)malloc(sizeof(char) * 1024);
    DIR* dir = NULL;
    struct dirent* entry = NULL;
    getcwd(cwd, 1024);//현재 디렉토리를cwd에 저장
    if ((dir = opendir(cwd)) == NULL)//현재 디렉토리를 열어준다.
    {
        printf("current directory error\n");
        exit(1);
    }
    struct stat* buf; //파일이나 디렉토리의 정보를 다 담고있음.
    while ((entry = readdir(dir)) != NULL)//해당디렉토리의 가장 앞에 있는 포인터를 반환
    {
        if (argc == 2)//ls 
        {
            if (entry->d_name[0] == '.') continue;
            printf("%s  ", entry->d_name);
        }
        else if (strcmp(op, "-a") == 0)//앞이 .으로 시작하는 숨겨진 파일을 모두 보여줌
        {
            printf("%s  ", entry->d_name);
        }
        else if (strcmp(op, "-l") == 0) //ls -l
        {
            if (entry->d_name[0] == '.') continue;
            stat(entry->d_name, buf);//퍼미션 정보를 출력
            printf("%c", S_ISDIR(buf->st_mode) ? 'd' : '-');
            printf("%c", buf->st_mode & S_IRUSR ? 'r' : '-');
            printf("%c", buf->st_mode & S_IWUSR ? 'w' : '-');
            printf("%c", buf->st_mode & S_IXUSR ? 'x' : '-');
            printf("%c", buf->st_mode & S_IRGRP ? 'r' : '-');
            printf("%c", buf->st_mode & S_IWGRP ? 'w' : '-');
            printf("%c", buf->st_mode & S_IXGRP ? 'x' : '-');
            printf("%c", buf->st_mode & S_IROTH ? 'r' : '-');
            printf("%c", buf->st_mode & S_IWOTH ? 'w' : '-');
            printf("%c\t", buf->st_mode & S_IXOTH ? 'x' : '-');
            printf("%s\t", getpwuid(buf->st_uid)->pw_name);//유저아이디를 출력
            printf("%s\t", getgrgid(buf->st_gid)->gr_name);//그룹아이디 출력
            printf("%ld\t", buf->st_nlink);//링크되어있는 숫자를 출력
            printf("%-6ld\t", buf->st_size);//파일의 사이즈 출력
            int i = 0;
            while (ctime(&buf->st_mtime)[i] != '\n') {//시간출력 마지막에 개행이 있어서 while로개행 전까지  출력함
                printf("%c", ctime(&buf->st_mtime)[i]);
                i++;
            }
            printf("\t%s\n", entry->d_name);
        }
        else if (strcmp(op, "-la") == 0 || strcmp(op, "-al") == 0)
        {
            stat(entry->d_name, buf);
            printf("%c", S_ISDIR(buf->st_mode) ? 'd' : '-');
            printf("%c", buf->st_mode & S_IRUSR ? 'r' : '-');
            printf("%c", buf->st_mode & S_IWUSR ? 'w' : '-');
            printf("%c", buf->st_mode & S_IXUSR ? 'x' : '-');
            printf("%c", buf->st_mode & S_IRGRP ? 'r' : '-');
            printf("%c", buf->st_mode & S_IWGRP ? 'w' : '-');
            printf("%c", buf->st_mode & S_IXGRP ? 'x' : '-');
            printf("%c", buf->st_mode & S_IROTH ? 'r' : '-');
            printf("%c", buf->st_mode & S_IWOTH ? 'w' : '-');
            printf("%c\t", buf->st_mode & S_IXOTH ? 'x' : '-');
            printf("%s\t", getpwuid(buf->st_uid)->pw_name);
            printf("%s\t", getgrgid(buf->st_gid)->gr_name);
            printf("%ld\t", buf->st_nlink);
            printf("%-6ld\t", buf->st_size);
            int i = 0;
            while (ctime(&buf->st_mtime)[i] != '\n') {
                printf("%c", ctime(&buf->st_mtime)[i]);
                i++;
            }
            printf("\t%s\n", entry->d_name);
        }
    }
    printf("\n");
    free(cwd);
    closedir(dir);
}

void PWD()//현재 디렉토리를 출력하는 함수.
{
    char wd[BUFSIZ];
    printf("%s\n", getcwd(wd, BUFSIZ));
}

void CP(char* name1, char* name2) {//name1을 name2로 복사해주는 함수.
    int fd1, fd2;
    ssize_t nread;
    char buffer[1024];

    if ((fd1 = open(name1, O_RDONLY)) < 0) {//file1을 읽기 모드로 열어줌
        perror(name1);
        exit(-1);
    }
    if ((fd2 = open(name2, O_WRONLY | O_CREAT | O_EXCL, PERM)) < 0) {
        fprintf(stderr, "파일 %s가 이미 존재합니다", name2);//file2를 읽기 쓰기 생성 모드로 열음
        exit(-1);
    }
    while ((nread = read(fd1, buffer, 1024)) > 0) {//fd1을 버퍼에 저장해서 fd2에 쓰기 
        if (write(fd2, buffer, nread) < nread) {
            break;
        }
    }

    close(fd1);
    close(fd2);
}

void mcp(int argc,char* argv[]){
    int num=argc-3;
    pthread_t p_thread[num];
    int thr_id;
    twoname* tn=(twoname*)malloc(sizeof(twoname)*num);
    for (int i = 0; i < num; i++)
    {
      tn[i].first=argv[2];
      tn[i].second=argv[i+3];
      thr_id = pthread_create(&p_thread[i], NULL, CP, (void*)&tn[i]);
      if (thr_id < 0)
      {
        perror("thread create error : ");
      }
    }
    for (int i = 0; i < num; i++)
    {
        pthread_join(p_thread[i], NULL);//스레드가 종료될떄까지 기다려줌
    }

}


int finding(char* name1, char* name2)//name1 이 name2 안에 포함되어있는지 확인하는 함수.
{
    int i = 0, j = 0;
    while (i < strlen(name1))
    {
        if (name1[i] == name2[j])
            j++;
        else
            j = 0;
        if (j == strlen(name2))
            return 1;
        i++;
    }
    return 0;
}

void Find(char* find, int ver, char* chname)//하위폴더까지 합쳐서 find이름을 가진 파일 또는 디렉토리가 있는지 출력하는 함수.
{//find: 찾고자 하는 파일 이름 ver: 서브디렉토리 인지 확인해주는 함수 chname: 서브디렉토리의 이름
    int t = 0;
    char* cwd = (char*)malloc(sizeof(char) * 1024);
    DIR* dir = NULL;
    struct dirent* entry = NULL;
    if (ver != 0)//서브디렉토리면 이름을 바꿔줌 
        chdir(chname);
    getcwd(cwd, 1024);
    if ((dir = opendir(cwd)) == NULL)
    {
        printf("current directory error\n");
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL)//. ..을 제외한 디렉토리가 나올경우 헤당  디렉토리로 이동.
    {
        if (entry->d_type == DT_DIR && (strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
        {
            if (finding(entry->d_name, find))
                printf("%s/%s\n", cwd, entry->d_name);
            Find(find, 1, entry->d_name);
        }
        else//이름이 동일한 경우 출력.
        {
            if (finding(entry->d_name, find))
                printf("%s/%s\n", cwd, entry->d_name);
        }
    }
    free(cwd);
    closedir(dir);
    if (ver == 1)
        chdir("..");
}

void* mRMdir(void* fname)
{
  char* filename=(char*)fname;
  if(rmdir(filename)==-1)
    fprintf(stderr, "%s directory remove error: %s\n", filename, strerror(errno));
}


void RMDIR(char* op, char* name)//딕셔너리를 삭제하는 함수.
{
    char path[1024];
    strcpy(path, name);
    if (strcmp(op, "-p") == 0) {//상위 디렉토리도 함께 삭제.
        int t = strlen(path);
        for (int i = t; i > -1; i--)
        {
            if (path[i] == '/')//디렉토리들을 나눠준다.
                path[i] = '\0';//
            if (path[i] == '\0') {
                if (rmdir(path) == -1) {
                    fprintf(stderr, "%s directory remove error: %s\n", path, strerror(errno));
                }
            }

        }
    }
    else if (rmdir(path) == -1) {
        fprintf(stderr, "%s directory remove error: %s\n", path, strerror(errno));
    }
}

void CHMOD(char* mod, char* filename)//접근권한을 바꿔주는 함수.
{
    struct stat* buf;
    int mode = (mod[0] - 48) * 64 + (mod[1] - 48) * 8 + (mod[2] - 48);//mod를 8진수 코드로 바꿔준다.
    chmod(filename, mode);
}

int main()
{
    char* cwd = (char*)malloc(sizeof(char) * 1024);
    getcwd(cwd, 1024);
    int argc;
    char* argv[15];
    char s1[100];
    while (1) {
        printf(" %s>>", cwd);
        int num = 1;
        rewind(stdin);
        scanf(" %[^\n]s", s1);
        char* ptr = strtok(s1, " ");
        while (ptr != NULL)
        {
            argv[num++] = ptr;
            ptr = strtok(NULL, " ");
        }
        argc = num;
        if (strcmp(argv[1], "exit") == 0) break;

        if (strcmp(argv[1], "ls") == 0)
        {
            myls(argc, argv[2]);
        }

        if (strcmp(argv[1], "mkdir") == 0)
        {
            if (argc > 3) {
                if (strcmp(argv[2], "-m") == 0)//접근권한을 본인이 설정할 수 있는 옵션
                {
                    Omkdir(argv[2], argv[4], argv[3]);
                }
                else if (strcmp(argv[2], "-p") == 0)
                {
                    Omkdir(argv[2], argv[3], "755");
                }
                else
                    Mmkdir(argc, argv);
            }
            else
                Omkdir("", argv[2], "755");
        }


        if (strcmp(argv[1], "cd") == 0)
        {
            chdir(argv[2]);
            getcwd(cwd, BUFSIZ);
        }
        if (strcmp(argv[1], "mv") == 0)
        {
            if (rename(argv[2], argv[3]) == -1) {
                print_error(argv[1], argv[2], argv[3]);
            }
        }
        if (strcmp(argv[1], "rmdir") == 0)
        {
            if (argc == 3)
                RMDIR("", argv[2]);
            else
                RMDIR(argv[2], argv[3]);
        }
        if (strcmp(argv[1], "pwd") == 0)
            PWD();
        if (strcmp(argv[1], "cp") == 0)
            CP(argv[2], argv[3]);
        if (strcmp(argv[1], "find") == 0)
            Find(argv[2], 0, ".");
        if (strcmp(argv[1], "chmod") == 0)
            CHMOD(argv[2], argv[3]);
        if (strcmp(argv[1], "rm") == 0)
            RM(argv[2]);
        if (strcmp(argv[1], "cat") == 0)
        {
            if (argc == 3)
                Cat(argc, "", argv[2]);
            if (argc == 4)
                Cat(argc, argv[2], argv[3]);
        }
    }
}