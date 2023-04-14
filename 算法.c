#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#define HIDWORD(l) ((__int32_t)(((__int64_t)(l) >> 32) & 0xFFFFFFFF))
FILE* fp = NULL;
FILE* fps1 = NULL;
FILE* fps2 = NULL;
FILE* fps3 = NULL;
FILE* fpzt = NULL;
FILE* fpkg = NULL;

long len1, len2;
int sc = 1;
int as = 1;
int ljxh;
char xh[100];
char L1[100] = "/sdcard/4/输出1.txt";
int zt;
int kg;
__int64_t count = 0;
__int64_t sss;

struct  _Base{
    __int64_t type;
    __int64_t offset;
    char buffer[52];
    char outputFilePath[202];
    __int32_t nodeLen;
    __int32_t nPointerSize[20];
    __int32_t nBaseSize[20];
    __int64_t Path[7];
};

typedef struct Link{
    __int64_t pointerSize;//0
    __int64_t *pointer1;//8
    __int64_t *pointer2;//10
    struct Link * prior; //18指向直接前趋
    struct Link * next; //20指向直接后继
}DNode;

//__int64_t count = 0;
DNode *Tail = NULL;

struct  _Base Base;

struct  _mod{
    __int64_t *m1;
    char (*m2)[50]
};

struct  _mod mod[11];

__int64_t initial_cfg(const char *filePath)
{
    __int64_t type = 0LL;
    __int32_t dwSize = 0;
    char s[200];

    FILE *stream;
    int flag = 1 ;

    int nindexPointerSize = 0;
    int nindexnBaseSize = 0;
    stream = fopen(filePath, "r");
    if ( stream == 0LL )
    {
        puts("初始化失败");
        exit(-1);
    }

    while ( fgets(&s, 200, stream) != 0 )
    {
        if ( flag )
        {
            sscanf(&s, "%d<>%lld<>%lld<>%s", &Base.nodeLen, &Base.type, &Base.offset,Base.outputFilePath);
            flag = 0;
        }
        else
        {
            sscanf(&s, "%d|%d|%d|", &type, (char *)&type + 4, &dwSize);

            if ( (__int32_t)type == 0x7D0 )
            {

                Base.nPointerSize[nindexPointerSize++] = dwSize;
            }
            else if ( (__int32_t)type == 0xDAC )
            {
               Base.nBaseSize[nindexnBaseSize++] = dwSize;
            }
        }
    }
}


DNode *  create_list(int length)
{
    DNode *head;
    DNode *node;
    int i;
    DNode* p;

    head = malloc(sizeof(DNode));
    p = head;
    if ( !head )
    {
        puts("创建链表失败！");
        exit(-1);
    }
    for ( i = 0; i < length; ++i )
    {
        node = malloc(sizeof(DNode));
        if ( !node )
        {
            puts("创建节点失败");
            exit(-1);
        }
        node->prior = p;
        p->next = node;
        p = node;
    }
    Tail = p;
    head->prior = NULL;
    p->next = NULL;
    return head;
}

char*  Allocate_memory(int count)
{
char (*p)[50] = (char(*)[50])malloc(count * 50 * sizeof(char));
    if ( !p )
    {
        puts("内存分配失败");
        exit(-1);
    }
    return p;
}

void  initial_memory(DNode *pTail)
{

    int i;
    DNode *p = pTail;
    for ( i = 0; ; ++i )
    {
        if ( i >= Base.nodeLen )
            break;
        p->pointer1 = (__int64_t *)malloc(8 * Base.nPointerSize[i]); //4c 2000

        p->pointer2 = (__int64_t *)malloc(8 * Base.nPointerSize[i]);

        p = p->prior;


        if (Base.nBaseSize[i] > 0 )
        {
            mod[i].m1= (__int64_t *)malloc(8LL * Base.nBaseSize[i]);//44 3000
            mod[i].m2 = Allocate_memory(Base.nBaseSize[i]);
        }
    }
}

__int64_t  initial_list(const char *filePath)
{

    char s[100];
    int k;
    int j;
    int i;

    FILE *stream;
    DNode *p= Tail;

    stream = fopen(filePath, "r");
    if ( stream == 0LL )
    {
        puts("初始化失败");
        exit(-1);
    }

    initial_memory(Tail);

    for ( i = 0; i < Base.nodeLen; ++i )
    {
        p->pointerSize = Base.nPointerSize[i];
        for ( j = 0; j < Base.nPointerSize[i]; ++j )
        {
            fgets(&s, 100, stream);
            sscanf(&s, "Pointer|%lld|%lld", p->pointer1+j, p->pointer2+j);

            if ( ((unsigned char)Base.type ^ 1) & 0xFF )
                *(p->pointer2 + 8LL * j) &= 0xFFFFFFFFuLL;
        }

        for ( k = 0; k < Base.nBaseSize[i]; ++k )
        {
            fgets(&s, 100, stream);
            sscanf(&s, "Base|%lld|%s", &mod[i].m1[k], (char *)mod[i].m2[k]);

            if ( ((unsigned char)Base.type ^ 1) & 0xFF )
                mod[i].m1[k] &= 0xFFFFFFFFuLL;
        }
        p = p->prior;
    }
    return fclose(stream);
}

void Print_route(int dwSize)
{
    int i;
    fputs(Base.buffer, fp);
    for ( i = 0; i < dwSize; ++i ) {
    fprintf(fp, "+0x%X", Base.Path[i]);
    }
    fputc('\n', fp);
    ++count;
}

void  Traversal_path(DNode *aTail,  unsigned  int aOffset,  unsigned int aIndex, __int64_t aValue)
{   
    unsigned  int t;
    unsigned  int  nOffset = aOffset;
    unsigned  int nIndex = aIndex;
    __int64_t nValue = aValue;
    __int64_t i;

    DNode *p = aTail;
    
    for ( i = 0; ; ++i )
    {
        if ( i >= p->pointerSize )
            break;
        
        __int64_t a = p->pointer1[i] - Base.offset;
        __int64_t b = p->pointer1[i];
        if ( nValue >= a   && nValue <= b  )
        {
            Base.Path[nOffset] = p->pointer1[i] - nValue;
            
            if ( p->next )
            {
                t = nOffset + 1;
                Traversal_path(p->next, t, nIndex, p->pointer2[i]);
                nOffset = t - 1;
            }
            else
            {
                Print_route(nIndex);
             }
             }
}
}

int main(int argc,char *argv[],char *envp[]) {
	fps1 = fopen(L1, "a");
    int i,j;
    float timeuse;

    struct timeval starttime,endtime;

    int bRet = strcmp(argv[2], "in");


    gettimeofday(&starttime,NULL);//开始

    initial_cfg("/sdcard/配置/[C]配置数据对接信号");
    DNode *head = create_list(Base.nodeLen);
    initial_list("/sdcard/配置/[C]链表数据DATA.txt");
    fp = fps1;
    DNode *p = Tail;
    for ( i = 0; i < Base.nodeLen; ++i )
    {
        for ( j = 0; j < Base.nBaseSize[i]; ++j )
        {
            strcpy(Base.buffer,mod[i].m2[j]);

            Traversal_path(p, 0,i + 1, mod[i].m1[j]);
        }
        p  = p->prior;
    }
    gettimeofday(&endtime, 0LL);
    timeuse = (1000000 * (endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec - starttime.tv_usec) / 1000000.0;
   FILE *stream = fopen(argv[1], "w");
   printf("耗时%lf\n", timeuse);
   printf("行数%ld\n", count);
   return 0;
}