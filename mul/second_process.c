#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>

#define errExit(msg)	do {	perror(msg);		\
								exit(EXIT_FAILURE);	\
						} while(0);

/* the second process will run first in the background */
int main(int argc, char** argv)
{
	int		fd;
	char	*shm_obj_name = "/shm_moumou";		/* shared memory object name */
	char    *sem1_obj_name = "/sem1_moumou";    /* semaphore1 object name */
	char    *sem2_obj_name = "/sem2_moumou";    /* semaphore2 object name */
	void	*shv;								/* a 8-byte size shared memory as a single long long shared variable */
	sem_t	*sem1;								/* sem1 is used to synchronize the order of these two processes */ 
	sem_t 	*sem2;								/* sem2 is used to guarantee the mutual exclusion of these two processes */

	/* create the shared memory	*/
	fd = shm_open(shm_obj_name, O_CREAT | O_RDWR | O_EXCL, 0666);
	if(fd == -1)	
		errExit("shm open by P2");

	/* set the size of the shared memory to 8-byte of long long type */
	if(ftruncate(fd, sizeof(long long)) == -1){
		close(fd);
		errExit("ftruncate by P2");
	}

	/* map the object into the second process's address space */
	shv = mmap(NULL, sizeof(long long), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if(shv == MAP_FAILED)
		errExit("mmap by P2");

	/* initialize the value of the shared variable to zero */
	*((long long*)shv) = 0;

	/* create the semaphore sem1 */
	sem1 = sem_open(sem1_obj_name, O_RDWR | O_CREAT, 0666, 0);
	if(sem1 == SEM_FAILED)
		errExit("sem1 create by P2");

	/* create the semaphore sem2 as a binary semaphore*/
	sem2 = sem_open(sem2_obj_name, O_RDWR | O_CREAT, 0666, 1);
	if(sem2 == SEM_FAILED)
		errExit("sem2 create by P2");

	/* blocked by sem1 and wait(be put to sleep) to be rescued by the first process */
	if(sem_wait(sem1) == -1)
		errExit("sem1 wait by P2");

	/* close the semaphore1 and decreament the semaphore1 reference count */
    if(sem_close(sem1) == -1) 
        errExit("sem1 close by P2");

	/* increament the shared variable by certain amount	*/
	for(long long i = 0; i < 100000; ++i){
		if(sem_wait(sem2) == -1) errExit("sem2 wait by P2");
		*((long long*)shv) = *((long long*)shv) + 1;
		if(sem_post(sem2) == -1) errExit("sem2 post by P2");
	}

	/* close the semaphore2 and decreament the semaphore2 reference count */
	if(sem_close(sem2) == -1)
		errExit("sem2 close by P2");

	/* display the result when the second process leaves */
	printf("shared variable finished by P2 : %lld\n", *((long long*)shv));

	/* Unmapped the shared memory object and if the reference  *
	 * count to it decreases to zero, the shared memory object *
	 * will be freed by kernel. 								   */
    munmap(shv, sizeof(long long));   

	exit(0);
}
