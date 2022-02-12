#include <pwd.h>
#include <err.h>
#include <shadow.h>
#include <crypt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bsd/readpassphrase.h>
#include "sessions.h"

#define VERSION "1.3"

void getconf(FILE* fp, const char* entry, char* result, size_t len_result) {
	char* line = NULL;
	size_t len = 0;

	fseek(fp, 0, SEEK_SET);

	while (getline(&line, &len, fp) != -1) {
		if (strncmp(entry, line, strlen(entry)) == 0) {
			strtok(line, "=");
			char* token = strtok(NULL, "=");
			if (token) {
				strncpy(result, token, len_result);
				result[strcspn(result, "\n")] = 0;
				free(line);
				return;
			}
		}
	}

	errx(1, "Could not get '%s' entry in config", entry);
}

int runprog(int argc, char** argv) {
	for(int i=0; i<argc; i++)
		argv[i] = argv[i + 1];

	if (setuid(0) < 0)
		err(1, "Could not setuid");
	if (setgid(0) < 0)
		err(1, "Could not setgid");

	if (execvp(argv[0], argv) != 0)
		perror(argv[0]);

	return 0;
}

int main(int argc, char** argv) {
	char username[64], wrong_pw_sleep[64], session_ttl[64], password[128];
	unsigned int sleep_ms, tries, ts_ttl;

	if (argc == 1) {
		printf("RootDO version: %s\n\n", VERSION);
		printf("Usage: %s [command]\n", argv[0]);
		return 0;
	}

	if (geteuid() != 0)
		errx(1, "The rdo binary needs to be installed as SUID.");

	int ruid = getuid();
	if (ruid == 0)
		return runprog(argc, argv);

	FILE* fp = fopen("/etc/rdo.conf", "r");

	if (!fp)
		err(1, "Could not open /etc/rdo.conf");

	getconf(fp, "username", username, sizeof(username));
	getconf(fp, "wrong_pw_sleep", wrong_pw_sleep, sizeof(wrong_pw_sleep));
	getconf(fp, "session_ttl", session_ttl, sizeof(session_ttl));
	sleep_ms = atoi(wrong_pw_sleep) * 1000;
	ts_ttl = atoi(session_ttl) * 60;

	fclose(fp);

	if (getsession(getppid(), ts_ttl) == 0)
		return runprog(argc, argv);

	struct passwd* p = getpwnam(username);
	if (!p) {
		if (errno == 0)
			errx(1, "The user specified in the config file does not exist.");
		else
			err(1, "Could not get user info");
	}

	int uid = p->pw_uid;
	if (uid != ruid && ruid != 0)
		errx(1, "You are not allowed to execute rdo.");

	struct spwd* shadowEntry = getspnam(p->pw_name);

	if (!shadowEntry || !shadowEntry->sp_pwdp)
		err(1, "Could not get shadow entry");

	tries = 0;
	while (tries < 3) {
		if (!readpassphrase("(rdo) Password: ", password, sizeof(password), RPP_REQUIRE_TTY))
			err(1, "Could not get passphrase");

		char* hashed_pw = crypt(password, shadowEntry->sp_pwdp);
		memset(password, 0, sizeof(password));
		
		if (!hashed_pw)
			err(1, "Could not hash password");

		if (strcmp(shadowEntry->sp_pwdp, hashed_pw) == 0) {
			setsession(getppid(), ts_ttl);
			return runprog(argc, argv);
		}

		usleep(sleep_ms);
		fprintf(stderr, "Wrong password.\n");
		tries++;
	}
	errx(1, "Too many wrong password attempts.");
	return 1;
}
