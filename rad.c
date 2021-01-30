/*
 * rad -- Run As Daemon
 */

/* Written by okole - github.com/okole */

/* HOW DOES IT WORK?
 *
 * There are a couple of important syscalls in this software
 *      * fork()
 *      * exec()
 *      * signal()
 *      * open()
 *
 * Step 1: Create a Daemon Process
 *      In order to create a daemon you need to
 *      1. Fork and create a child process
 *      2. Remove parent process
 *      3. Assign new process group as old child as leader
 *
 * Step 2:
 *      Redirect STDOUT
 *      1. Delete all file descriptors
 *      2. Open 1, 2, and 3 file descriptors on the file you want
 *
 * Step 3:
 *      The exec* syscall suite
 *      THINGS TO NOTE: Exec will write over memory with the new processes memory,
 *      but it will not change what the kernal knows about the original process,
 *      including file descriptors, signal ignores, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define PROGRAMNAME "rad"

#define ERRQ(msg, ...) {\
    fprintf(stderr, msg "\n", ##__VA_ARGS__); \
    exit(EXIT_FAILURE); \
}

#define TRUE 1
#define FALSE 0

static void usage() {
    printf("rad [ARGS] --exec COMMAND [ARGS]\n");
    exit(EXIT_SUCCESS);
}

static void redirectstdio(const char * logfile) {
    if(logfile == NULL) logfile = "/dev/null";
    else remove(logfile);

    for(int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) close(x);

    if(open("/dev/null", O_RDWR) != 0) syslog(LOG_ERR, "Failed to redirect stdin");
    if(open(logfile, O_RDWR) != 1) {
        if(open(logfile, O_RDWR | O_CREAT, 0555) != 1)
            syslog(LOG_ERR, "Failed to redirect stdout");
    }

    (void) dup(1);
}

static void makedaemon(const unsigned char nohup, const char *logfile) {
    pid_t pid = 0;

    if((pid = fork()) == -1) ERRQ("Failed to fork()!");

    if(pid > 0) exit(EXIT_SUCCESS);    

    openlog(PROGRAMNAME, LOG_PID | LOG_NDELAY, LOG_DAEMON);

    if(setsid() == -1) {
        syslog(LOG_ERR, "Failed to set session leader!");
        exit(EXIT_FAILURE);
    }

    closelog();

    if((pid = fork()) == -1) ERRQ("Failed to fork()!");

    if(pid > 0) exit(EXIT_SUCCESS);

    redirectstdio(logfile);

    openlog(PROGRAMNAME, LOG_PID | LOG_NDELAY, LOG_DAEMON);

    if(setsid() == -1) {
        syslog(LOG_ERR, "Failed to set session leader!");
        exit(EXIT_FAILURE);
    }

    if(signal(SIGCHLD, SIG_IGN) == SIG_ERR) syslog(LOG_ERR, "Failed to set signal ignore!");
    if(nohup == TRUE) {
        if(signal(SIGHUP, SIG_IGN) == SIG_ERR) syslog(LOG_ERR, "Failed to set signal ignore hup!");
    }
}

int main(int argc, char **argv, char **envp) {
    puts("rad - Daemon Utility");

    opterr = 0;
    unsigned char nohup = FALSE;
    unsigned char execflag = FALSE;
    unsigned int offset = 1;

    const char *logfile = NULL;
    const char *directory = NULL;

    if(argc < 2) usage();

    for(int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "--exec") == 0) {
            execflag = TRUE;
            break;
        } else if(strcmp(argv[i], "-n") == 0) {
            nohup = TRUE;
        } else if(strcmp(argv[i], "-o") == 0) {
            logfile = argv[++i];
            offset++;
        } else if(strcmp(argv[i], "-d") == 0) {
            directory = argv[++i];
            offset++;
        }

        offset++;
    }

    if(execflag == FALSE) ERRQ("Failed to find exec!");

    makedaemon(nohup, logfile);

    if(directory != NULL) {
        if(chdir(directory) == -1) syslog(LOG_ERR, "Failed to chdir!()");
    }

    char **cmd = argv + offset;

    for(int i = 0; cmd[i] != NULL; i++) {
        syslog(LOG_ERR, "%s", cmd[i]);
    }

    if(execve(*cmd, cmd, envp) == -1) syslog(LOG_ERR, "Failed to exec cmd! %s", strerror(errno));

    closelog();

    fflush(stdout);
    fflush(stderr);

    exit(EXIT_SUCCESS);
}
