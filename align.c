/**
 * @file    main.c
 * @brief   TODO brief documentation here.
 *
 * @author
 * @version $Id$
 */

/*{{{ Headers ----------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
/*---------------------------------------------------------------- Headers }}}*/

void sig_handler(int signo)
{
    int stop = 0;
    static char *s = "Signal handler called!";

    fprintf(stderr, "%s: %d, Waiting %d\n", s, signo, getpid());

    while (!stop)
        usleep(100000);

    exit(1);
} /* sig_handler */

void set_AC(void)
{
    int flags, newflags;

    /* stolen from /usr/src/linux-2.4.20-8/arch/i386/kernel/head.S */
    __asm__(
        "pushfq            # push EFLAGS\n\t"
        "pop %%rcx         # get EFLAGS\n\t"
        "mov %%rcx,%%rax   # save original EFLAGS\n\t"
        "or $0x40000,%%rcx # turn on AC bit in EFLAGS\n\t"
        "push %%rcx        # copy to EFLAGS\n\t"
        "popfq             # set EFLAGS\n\t"
        "pushfq            # get new EFLAGS\n\t"
        "popfq             # put it into newflags\n\t"
        : "=g" (flags), "=g" (newflags) : "r" (0)
    );

#if 0
    __asm__("pushf\n"
            "orl $0x40000, (%rsp)\n"
            "popf");
#endif
#if 0
        "pushfl          # push EFLAGS\n\t"
        "popl %2         # get EFLAGS\n\t"
        "movl %2,%0      # save original EFLAGS\n\t"
        "orl $0x40000,%2 # turn on AC bit in EFLAGS\n\t"
        "pushl %2        # copy to EFLAGS\n\t"
        "popfl           # set EFLAGS\n\t"
        "pushfl          # get new EFLAGS\n\t"
        "popl %1         # put it into newflags\n\t"
        : "=g" (flags), "=g" (newflags) : "r" (0)
#endif

    /* fprintf(stderr, "EFLAGS old = 0x%08x new = 0x%08x\n", flags, newflags); */
} /* set_AC */

/* sizeof 는 align이 맞은 상태 */
#define ALIGNX(n, m)  (((n) + (m - 1)) & ~(m - 1))
#if 0
#define ALIGN(n)  ALIGNX(n, 4)
#endif
#define ALIGN(n)  ALIGNX(n, 8)
int align(int size) { return (size + 7) & ~(7); }

#define READ 0
#define WRITE 1

/*{{{ serialize test ---------------------------------------------------------*/
/* char * alignment */
typedef struct { int a; char *b; } val_t;
typedef struct { int w; val_t *v; } test_t;
const int count = 2;
#if 0
/* align 안맞음.. */
char *data[]  = {
#endif
char data[][8]  = {
    "Hello", "World!"
};
int type = 0;

void print(test_t *t, int is_child) {
    int i = 0;

    if (is_child == 0)
        fprintf(stderr, "Parent\n");
    else
        fprintf(stderr, "Child\n");

    fprintf(stderr, "t.w: %d\n", t->w);
    for ( ; i < count; i++) {
        fprintf(stderr, "t.v[%d]:\n", i);
        fprintf(stderr, "  a: %d\n", t->v[i].a);
        fprintf(stderr, "  b: %s\n", t->v[i].b);
    }
}

void set(test_t *t) {
    int i = 0;

    t->w = count;
    t->v = malloc(count * sizeof(val_t));
    for ( ; i < count; i++) {
        t->v[i].a = i;
        t->v[i].b = data[i];
#if 0
        /* 아래 가능, strcpy 는 align 때문에 안됨 */
        t->v[i].b = data[i];
        t->v[i].b = malloc(strlen(data[i]) + 1);
        memcpy(t->v[i].b, data[i], ALIGN(strlen(data[i]) + 1));

        strcpy(t->v[i].b, data[i]);
#endif
    }
}

int get_size() {
    int i = 0;
    int size = sizeof(test_t);

    size += count * sizeof(val_t);
    if (type == 0)
        for ( ; i < count; i++)
            /* +1 for '\0' for each data */
            size += ALIGN(strlen(data[i]) + 1);
    else {
        size += count;
        for ( ; i < count; i++)
            /* +1 for 'len' for each data */
            size += strlen(data[i]) + 1;
    }

    return ALIGN(size);
}

void serialize(char *buf, test_t *t) {
    int i = 0;

    memcpy(buf, t, sizeof(test_t));
    buf += sizeof(test_t);

    for ( ; i < count; i++) {
        memcpy(buf, &t->v[i], sizeof(val_t));
        buf += sizeof(val_t);
    }

    /**
     * | w | padding | v1 | v2 | v1.b + padding | v2.b + padding |
     **/
    if (type == 0) {
        for (i = 0; i < count; i++) {
            int aligned_len = ALIGN(strlen(t->v[i].b) + 1);
            memcpy(buf, t->v[i].b, aligned_len);
            buf += aligned_len;
        }
    }
    /**
     * | w | padding | v1 | v2 | strlen(v1.b) | strlen(v2.b) | v1.b + v2.b |
     * strlen < 256
     **/
    else {
        char *data_pos = buf + count;
        for (i = 0; i < count; i++) {
            int j = 0;
            int len = strlen(t->v[i].b);

            /* no null terminator */
            *buf = len;
            for ( ; j < len; j++)
                data_pos[j] = t->v[i].b[j];
#if 0
            /* 아래 같은 memcpy 는 data_pos의 align 이 안맞아서 안된다. */
            memcpy(data_pos, t->v[i].b, ALIGN(len + 1));
            memset(data_pos + len, 0x00, ALIGN(len) - len);
#endif

            buf++;
            data_pos += len;
        }
    }
}

void deserialize(char *buf, test_t *new) {
    int i = 0;

    memcpy(new, buf, sizeof(test_t));
    buf += sizeof(test_t);

    new->v = malloc(count * sizeof(val_t));
    for (i = 0; i < count; i++) {
        memcpy(&new->v[i], buf, sizeof(val_t));
        buf += sizeof(val_t);
    }

    if (type == 0)
        for (i = 0; i < count; i++) {
            int len = strlen(buf);
            int aligned_len = ALIGN(len + 1);

            new->v[i].b = malloc(len + 1);
            memcpy(new->v[i].b, buf, aligned_len);
            new->v[i].b[len] = '\0';
            buf += aligned_len;
        }
    else {
        char *data_pos = buf + count;

        for (i = 0; i < count; i++) {
            int j = 0;
            int len = *buf;

            new->v[i].b = malloc(len + 1);
            for ( ; j < len; j++)
                new->v[i].b[j] = data_pos[j];
#if 0
            memcpy(new->v[i].b, data_pos, ALIGN(len + 1));
#endif
            new->v[i].b[len] = '\0';

            buf++;
            data_pos += len;
        }
    }
}

int test_main(int argc)
{
    int     fd[2], nbytes, rc = 0;
    pid_t   pid;

    if ((rc = pipe(fd)) < 0) {
        fprintf(stderr, "Creating Pipe is Error [%d]\n", rc);
    }

    if((pid = fork()) == -1) {
        perror("fork");
        return 0;
    }

    if (pid == 0) {
        test_t t;
        int size = 0;
        char *msg;

        /* 자식 프로세스는 Write할꺼기에 Read FD는 닫아준다 */
        close(fd[READ]);

        set_AC();

        if (argc == 3) {
            int stop = 0;
            fprintf(stderr, "Waiting child %d\n", getpid());
            while (!stop)
                usleep(100000);
        }

        /* data */
        set (&t);
        size = get_size();
        msg = malloc(size);
        memset(msg, 0x00, size);
        serialize (msg, &t);

        print(&t, 1);

        /* Pipe에 메시지 보내기 */
        write(fd[WRITE], msg, size);
        free(msg);
        return 0;
    } else {
        char buf[128];
        test_t t;

        /* 부모 프로세스는 Read할꺼기에 Write FD는 닫아준다 */
        close(fd[WRITE]);

        set_AC();

        if (argc == 3) {
            int stop = 0;
            fprintf(stderr, "Waiting parent %d\n", getpid());
            while (!stop)
                usleep(100000);
        }

        /* Pipe에서 메시지 읽기 */
        nbytes = read(fd[READ], buf, sizeof(buf));

        deserialize (buf, &t);

        fprintf(stderr, "Received Parent: [%d]\n", nbytes);
        print(&t, 0);
    }
    return 0;
}
/*--------------------------------------------------------- serialize test }}}*/

/*{{{ serialize test 2 -------------------------------------------------------*/
typedef struct { int16_t x[3]; } t1_t;
typedef struct { int32_t y; } t2_t;

int test_main2(int argc)
{
    int     fd[2], nbytes, rc = 0;
    pid_t   pid;

    if ((rc = pipe(fd)) < 0) {
        printf("Creating Pipe is Error [%d]\n", rc);
    }

    if((pid = fork()) == -1) {
        perror("fork");
        return 0;
    }

    if (pid == 0) {
        t1_t v1;
        t2_t v2;
        int size1 = sizeof(t1_t);
#if 0
        int size1 = ALIGN(sizeof(t1_t));
#endif
        int size2 = sizeof(t2_t);
        int size = size1 + size2;
        char *msg;

        set_AC();

        return 0;

        /* 자식 프로세스는 Write할꺼기에 Read FD는 닫아준다 */
        close(fd[READ]);

        if (argc == 3) {
            int stop = 0;
            printf("Waiting child %d\n", getpid());
            while (!stop)
                usleep(100000);
        }

        /* data */
        v1.x[0] = 1;
        v1.x[1] = 2;
        v1.x[2] = 3;
        v2.y = 3.14;

        msg = malloc(size);
        memset(msg, 0x00, size);

        memcpy(msg, &v1, size1);
        memcpy(msg + size1, &v2, size2);

        printf("%d %d %d\n", v1.x[0], v1.x[1], v1.x[2]);
        printf("%d\n", v2.y);

        /* Pipe에 메시지 보내기 */
        write(fd[WRITE], msg, size);
        free(msg);
        return 0;
    } else {
        char buf[128];
        t1_t *v1;
        t2_t *v2;
        int size1 = sizeof(t1_t);
#if 0
        int size1 = ALIGN(sizeof(t1_t));
#endif

        /* 부모 프로세스는 Read할꺼기에 Write FD는 닫아준다 */
        close(fd[WRITE]);

        if (argc == 3) {
            int stop = 0;
            printf("Waiting parent %d\n", getpid());
            while (!stop)
                usleep(100000);
        }

        /* Pipe에서 메시지 읽기 */
        nbytes = read(fd[READ], buf, sizeof(buf));

        v1 = (t1_t *)buf;
        v2 = (t2_t *)(buf + size1);
#if 0
        /* memcpy는 문제 발생 안함 */
        memcpy(&v1, buf, size1);
        memcpy(&v2, buf + size1, size2);
#endif

        printf("%d %d %d\n", v1->x[0], v1->x[1], v1->x[2]);
        printf("%d\n", v2->y);

        printf("Received Parent: [%d]\n", nbytes);
    }
    return 0;
}
/*------------------------------------------------------- serialize test 2 }}}*/

int main(int argc, char **argv) {
    if (argc > 1 && argv[1] != NULL)
        type = atoi(argv[1]);

    signal(SIGBUS, sig_handler);

    return test_main(argc);
#if 0
    return test_main2(argc);
#endif
}

#if 0
#if 0
char *str[] = { "de", "fg" };
char *str2[] = { "fg" };
#endif
char *str="de";
char *str2="fg";

int main(void)
{
    char *dest, *src;

    __asm__("pushf\n"
            "orl $0x40000, (%rsp)\n"
            "popf");

    dest = malloc(8);
    src = malloc(8);

#if 0
    /* success */
    memcpy(dest, src, 4);
    memcpy(dest, "ab", 4);
    strcpy(dest, "ab");
    strcpy(dest, str);
#endif
#if 0
    /* failed
     * 최초 선언된 string 말고는 다 align 에러 나는 듯.. */
    memcpy(dest, src, 5);
    strcpy(dest, str2);
#endif

    fprintf(stderr, "%s\n", dest);
    return 0;
}
#endif

/* end of main.c */
