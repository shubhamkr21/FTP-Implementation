#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <sys/wait.h>

int getcase(char str[])
{
	if(strcmp(str, "put") == 0)
		return 1;
	if(strcmp(str, "get") == 0)
		return 2;
	if(strcmp(str, "ls") == 0)
		return 3;
	if(strcmp(str, "cd") == 0)
		return 4;
	if(strcmp(str, "pwd") == 0)
		return 5;
	if(strcmp(str, "quit") == 0)
		return 6;
	return 0;
}

int create_socket(int port,char *addr)
{
	int sockfd;
	struct sockaddr_in servaddr;
	sockfd = socket (AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		printf("Problem in creating the socket\n");
		exit(2);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(addr);
	servaddr.sin_port =  htons(port); //convert to big-endian order

	//Connection of the client to the socket
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
		printf("Problem in creating data channel\n");
		exit(3);
	}
	return(sockfd);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in server, client;
	struct stat obj;
	int sockfd, newsockfd;
	char buf[100], command[10], filename[20];
	int k, i, size, c, choice;
	socklen_t len;
	pid_t chpid;
	FILE* file;

	if (argc < 2) {
		fprintf(stderr,"ERROR, Usage %s <PORT>\n",argv[0]);
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("Socket failed");
	  	exit(1);
	}
	server.sin_port = atoi(argv[1]);
	server.sin_addr.s_addr = INADDR_ANY;
	k = bind(sockfd,(struct sockaddr*)&server,sizeof(server));
	if(k == -1)
	{
		printf("Binding error\n");
	  	exit(1);
	}
	k = listen(sockfd,1);
	if(k == -1)
	{
	  printf("Listen failed");
	  exit(1);
	}
	else
		printf("Listening... waiting for connections\n");

	i = 1;
	while(1)
	{
		len = sizeof(client);
		newsockfd = accept(sockfd, (struct sockaddr*)&client, &len);
		//printf("Received request...\n");
		
		chpid = fork();
		if(chpid == 0) // child process
		{
			printf("Child created for dealing with client requests\n");
			close (sockfd);
			char port[6];
			read(newsockfd, port, 6);
		    printf("data port - %s\n",port);
		    int dport=atoi(port);
			char cliaddr[INET_ADDRSTRLEN];
		    inet_ntop(AF_INET, &client.sin_addr.s_addr, cliaddr, sizeof(cliaddr));
		    printf("cliaddr - %s\n", cliaddr);
			int datasockfd;
			datasockfd = create_socket(dport, cliaddr);

			while(1)
			{
				k = read(newsockfd, buf, 100);
				//printf("k-%d\n", k);
				if(k <= 0)
					break;
			  	sscanf(buf, "%s", command);
			  	printf("Recieved command - %s\n",command );
			  	choice = getcase(command);
				char port[6];

			  	switch(choice)
			  	{
			  		case 1:
				  		sscanf(buf + strlen(command), "%s", filename);
				  		printf("filename to save is %s\n", filename);
				  		char filearray[10241];
				  		read(newsockfd, &size, sizeof(int));
				  		i = 1;
				  		int recv_size=0;
					  	while(1)
					    {
					    	file = fopen(filename, "wb");
					      	if(file == NULL) 
							{
						  		sprintf(filename + strlen(filename), "%d", i);
							}
					      	else
								break;
				    	}
				  		int write_size, read_size;
				  		while(recv_size < size) {

					        do{
					              read_size = read(datasockfd,filearray, 10241);
					        } while(read_size < 0);
					        if(read_size == 0)
					            break;
					        write_size = fwrite(filearray,1,read_size, file);

					        if(read_size !=write_size) {
					            printf("error in read write\n");    
					        }
					        recv_size += read_size;
					        c = 1;
					    }
				  		fclose(file);
				  		write(newsockfd, &c, sizeof(int));
				  		printf("file downloaded succesfully\n");
				  		break;

				  	case 2:
				  		sscanf(buf + strlen(command), "%s", filename);
				  		printf("Filename to send is %s\n", filename);
				  		char send_buffer[10240];
						FILE* fp;
						fp = fopen(filename,"rb");
						if(fp != NULL)
						{
							write(newsockfd, "1" ,2);
							fseek(fp, 0, SEEK_END);
						    size = ftell(fp);
						    fseek(fp, 0, SEEK_SET);
						    printf("Total file size: %i\n",size);
						    write(newsockfd, &size, sizeof(int));
						  	int read_size, stat;
						  	while(!feof(fp))   
						    {
						        read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, fp);
						        do {
						     	   stat = write(datasockfd, send_buffer, read_size);
						        } while (stat < 0);
						        bzero(send_buffer, sizeof(send_buffer));
						    }
						    fclose(fp);
						  	printf("file sent\n");
						}
						else
						{
							write(newsockfd, "0" ,2);
						}
						break;

					case 3:
						c = 1;
						DIR* dirp;
					    struct dirent* direntp;
						char* cwd1;
						char buff1[PATH_MAX + 1];
						cwd1 = getcwd( buff1, PATH_MAX + 1);
					    dirp = opendir(cwd1);
					    if( dirp != NULL ) 
					    {
					        while(1) 
					        {
					            direntp = readdir( dirp );
					            if( direntp == NULL ) 
					            	break;
					            if(strcmp(direntp->d_name,"..") != 0 && strcmp(direntp->d_name,".") != 0)
					            {
					            	printf( "%s\n", direntp->d_name );
					            	write(datasockfd,"1",2);
									write(datasockfd, direntp->d_name, 1000);
					            }
					        }
					        write(datasockfd,"0",2);
					        closedir( dirp );
					    }
						break;

					case 4:
						if(chdir(buf+3) == 0)
			    			c = 1;
			  			else
			    			c = 0;
			        	send(newsockfd, &c, sizeof(int), 0);
			        	break;

			        case 5:
				  		i = 0;
				  		char* cwd;
	  					char buff[PATH_MAX + 1];
	  					cwd = getcwd( buff, PATH_MAX + 1);
	  					printf("cwd - %s\n", cwd);
	  					write(newsockfd, cwd, 100);
				      	break;

				    case 6:
				    	printf("FTP server quitting..\n");
				  		i = 1;
				  		send(newsockfd, &i, sizeof(int), 0);
				  		exit(0);
				  		break;
				  	default :
				  		printf("Invalid command\n");
			  	}
			}
			close(datasockfd);
	  	}
	}
	return 0;
}
