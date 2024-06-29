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

/*	This is the first process and it will run after the second process runs. */
int main(int argc, char** argv)
{
	int		fd;
	char	*shm_obj_name = "/shm_moumou";		/* shared memory object name */
	char	*sem1_obj_name = "/sem1_moumou";	/* semaphore1 object name */
	void	*shv;								/* a 8-byte size shared memory as a single long long shared variable */
	sem_t	*sem1;

	/* create the shared memory	*/
	fd = shm_open(shm_obj_name, O_RDWR, 0);
	if(fd == -1)	
		errExit("shm open by P1");

	/* set the size of the shared memory to 8-byte of long long type */
	if(ftruncate(fd, sizeof(long long)) == -1){
		close(fd);
		errExit("ftruncate by P1");
	}

	/* map the object into the first process's address space */
	shv = mmap(NULL, sizeof(long long), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if(shv == MAP_FAILED)
		errExit("mmap by P1");

	/* open the semaphore sem1 */
	sem1 = sem_open(sem1_obj_name, O_RDWR);
	sem_unlink(sem1_obj_name);					/* The name of the semaphore1 object is no longer needed, but the semaphore1 object still exist and will be closed later. */
	if(sem1 == SEM_FAILED)
		errExit("sem1 open by P1");

	/* increament the sem1(wake up the blocked second process) and release the freedom of the second process */
	if(sem_post(sem1) == -1)
		errExit("sem1 post by P1");

	/* close the semaphore1 and decreament the semaphore1 reference count */
	if(sem_close(sem1) == -1)
		errExit("sem1 close by P1");

	/* increament the shared variable by certain amount */
	for(long long i = 0; i < 100000; ++i)
        *((long long*)shv) = *((long long*)shv) + 1;

	printf("shared variable finished by P1 : %lld\n", *((long long*)shv));

	/* The name of the shared memory object is no longer needed, but the shared memory object still exist and will be unmapped later */
	shm_unlink(shm_obj_name);

	/* Unmapped the shared memory object and if the reference count to it decreases to zero, the shared memory object will be freed by kernel */
	munmap(shv, sizeof(long long));

	exit(0);
}
