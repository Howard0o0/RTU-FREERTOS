#include "memoryleakcheck.h"
#include "FreeRTOS.h"
#include "task.h"


/**
* 定义链表节点，表示一个内存泄漏信息
*/
typedef struct _mem_node
{
void *ptr; // 泄漏内存地址
size_t block; // 泄漏内存大小
size_t line; // 泄露发生的代码行
char *filename; // 泄漏发生的文件名
struct _mem_node *next; // 下一个节点指针
} mem_node;


// 取消malloc,  free的宏定义
// #undef pvPortMalloc
// #undef vPortFree


// 定义指向头节点的指针
mem_node *head = NULL;

/**
* 产生一个节点并加入链表
* @param ptr 分配的内存地址
* @param block 分配的内存单元大小
* @param line 代码行号
* @param filename 文件名称
*/
void mem_node_add(void *ptr, size_t block, size_t line, char *filename)
{
        // 产生节点
        mem_node *node = (mem_node *)pvPortMalloc(sizeof(mem_node));
        node->ptr = ptr;
        node->block = block;
        node->line = line;
        node->filename = filename;
        node->next = NULL;

        // 加入链表头节点
        if (head)
        {
                node->next = head;
                head = node;
        }
        else
                head = node;
}

/**
* 从链表中删除一个节点
* @param ptr 分配的内存地址
*/
void mem_node_remove(void *ptr)
{
        // 判断头节点是否存在
        if (head)
        {
                // 处理头节点
                if (head->ptr == ptr)
                {
                        // 获取头节点的下一个节点
                        mem_node *pn = head->next;
                        // 删除头节点
                        vPortFree(head);
                        // 令头节点指针指向下一个节点
                        head = pn;
                }
                else // 判断链表是否为空
                {
                        // 指向节点的指针
                        mem_node *pn = head->next;
                        // 指向前一个节点的指针
                        mem_node *pc = head;
                        // 遍历所有节点
                        while (pn)
                        {
                                // 获取指向下一个节点的指针
                                mem_node *pnext = pn->next;
                                if (pn->ptr == ptr)
                                {
                                        pc->next = pnext; // 删除当前节点
                                        vPortFree(pn);
                                }
                                else
                                        pc = pc->next;
                                pn = pnext;  
                                        
                        }                       
                }
        }
}

/**
* 显示内存泄漏信息
*/
void show_block()
{
        if (head)
        {
                // 保存总内存泄漏数量
                size_t total = 0;
                // 指向头节点的指针
                mem_node *pn = head;

                // 输出标题
                printf("\n\n--------memory leak report--------\n");

                // 遍历链表
                while (pn)
                {
                        mem_node *pnext = pn->next;
                        // 处理文件名
                        char *pfile = pn->filename, *plast = pn->filename;
                        while (*pfile)
                        {
                                // 找到\字符
                                if (*pfile == '\\')
                                plast = pfile + 1; // 获取\字符的位置
                                pfile++;
                        }
                        // 输出内存泄漏信息
                        printf("file:%s(%d), addr:%p(%dbyte)\n", plast, pn->line, pn->ptr, pn->block);
                        // 累加内存泄漏总量
                        total += pn->block;
                        // 删除链表节点
                        vPortFree(pn);
                        // 指向下一个节点
                        pn = pnext;
                }
                printf("total memory leak:%dbyte\n", total);
                head=NULL;
        }
}

/**
* 用于调试的malloc函数
* @param elem_size 分配内存大小
* @param filename 文件名称
* @param line 代码行号
*/
void *dbg_malloc(size_t elem_size, char *filename, size_t line)
{
        void *ptr = (void *)pvPortMalloc(elem_size);
        // 将分配内存的地址加入链表
        mem_node_add(ptr, elem_size, line, filename);
        return ptr;
}

/**
* 用于调试的free函数
* @param ptr 要释放的内存地址
*/
void dbg_free(void *ptr)
{
        vPortFree(ptr);
        // 从链表中删除节点
        mem_node_remove(ptr);
}