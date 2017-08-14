#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int getcase(char str[])
{
	if(strcmp(str, "fput") == 0)
		return 1;
	if(strcmp(str, "fget") == 0)
		return 2;
	if(strcmp(str, "servls") == 0)
		return 3;
	if(strcmp(str, "servcd") == 0)
		return 4;
	if(strcmp(str, "servpwd") == 0)
		return 5;
	if(strcmp(str, "clils") == 0)
		return 6;
	if(strcmp(str, "clicd") == 0)
		return 7;
	if(strcmp(str, "clipwd") == 0)
		return 8;
	if(strcmp(str, "quit") == 0)
		return 9;
	return 0;
}
int create_socket(int port)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	sockfd = socket (AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		printf("Problem in creating the socket\n");
		exit(2);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	if ((bind (sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) <0) {
		printf("Problem in binding the data socket\n");
		exit(2);
	}
 	listen (sockfd, 1);

	return(sockfd);
}

int accept_connect(int sock)
{
	int newsockfd;
	socklen_t clilen;
	struct sockaddr_in cliaddr;

	clilen = sizeof(cliaddr);
	if ((newsockfd = accept (sock, (struct sockaddr *) &cliaddr, &clilen)) <0) {
		printf("Problem in accepting the data socket\n");
		exit(2);
	}

	return(newsockfd);
}


int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct stat obj;
	int sockfd;
	int choice;
	char chstr[10];
	char buf[100], command[5], filename[30];
	char *f;
	int k, size, status;
	if (argc != 4) {
		fprintf(stderr,"ERROR, Usage %s <Server IP> <PORT> <Data Port>\n",argv[0]);
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("socket creation failed\n");
	  	exit(1);
	}
	server.sin_family = AF_INET;
	server.sin_port = atoi(argv[2]);
	server.sin_addr.s_addr = inet_addr(argv[1]);

	k = connect(sockfd, (struct sockaddr*)&server, sizeof(server));
	if(k == -1)
	{
		printf("Connect Error\n");
	  	exit(1);
	}
	int dport, datasockfd;
	char port[6];
	dport = atoi(argv[3]);
	datasockfd = create_socket(dport);	
	sprintf(port,"%d",dport);
	printf("sending data port - %s\n", port);
	write(sockfd, port, 6);
	datasockfd = accept_connect(datasockfd);
	int i = 1;
	while(1)
	{
		printf("myftp> ");
	  	scanf("%s", chstr);
	  	choice = getcase(chstr);
	    char port[6];
	  	switch(choice)
		{
			case 1:
			  	printf("Enter filename to upload a file to server: ");
			    scanf("%s", filename);
			    strcpy(buf, "put ");
			  	strcat(buf, filename);
			  	char send_buffer[10240];
				FILE* file;
			  	file = fopen(filename, "rb");
			    if(file == NULL)
		        {
		          	printf("%s: no such file on client\n", filename);
		          	break;
		        }
			  	write(sockfd, buf, 100);
			  	fseek(file, 0, SEEK_END);
			    size = ftell(file);
			    fseek(file, 0, SEEK_SET);
			    printf("Total file size: %i\n",size);
			    write(sockfd, &size, sizeof(int));
			    printf("file size sent\n");
			  	int read_size, stat;
			  	while(!feof(file))   
			    {
			        read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, file);
			        do {
			     	   stat = write(datasockfd, send_buffer, read_size);
			        } while (stat < 0);
			        bzero(send_buffer, sizeof(send_buffer));
			    }
			  	printf("file sent\n");
			  	fclose(file);
			  	read(sockfd, &status, sizeof(int));
			  	if(status)
			    	printf("File uploaded successfully\n");
			  	else
			    	printf("File failed to be upload to server\n");
			  	break;

			case 2:
				printf("Enter filename to download from the server: ");
				scanf("%s", filename);
				strcpy(buf, "get ");
				strcat(buf, filename);
			  	write(sockfd, buf, 100);
			  	char filearray[10241];
				FILE* fp;
				char stats[2];
				read(sockfd,&stats,2);
				if(strcmp("1", stats) == 0)
				{
				  	read(sockfd, &size, sizeof(int));
				  	printf("Total size of file - %d\n", size);
				  	if(size == 0)
				    {
				      	printf("%s: no such file on server\n",filename);
				    	break;
				    }
				  	int recv_size=0;
				  	while(1)
				    {
				      	fp = fopen(filename, "wb");
				      	if(fp == NULL)
						{
						  	sprintf(filename + strlen(filename), "%d", i);
						  	i++;
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
				        write_size = fwrite(filearray,1,read_size, fp);

				        if(read_size != write_size) {
				            printf("error in read write\n");    
				        }
				        recv_size += read_size;
				        printf("recieve size - %d\n", recv_size);
				    }
				  	fclose(fp);
				  	printf("File successfully downloaded\n");
				}
				else
					printf("Error in downloading file\n");
			  	break;
			
			case 3:
			  	strcpy(buf, "ls");
			    write(sockfd, buf, 100);
				char check[2] = "1";
				char buff[1000];
				while(strcmp("1",check) == 0)
				{ 
					read(datasockfd, check, 2);
					if(strcmp("0", check) == 0)
						break;
					read(datasockfd, buff, 1000);
					
					printf("%s  ",buff);
				}
				printf("\n");
			  	break;
			
			case 4:
			  	strcpy(buf, "cd ");
			  	char dir[96];
			  	//printf("Enter the path to change the server directory: ");
			  	scanf("%s", dir);
				strcat(buf, dir);
			    write(sockfd, buf, 100);
			  	read(sockfd, &status, sizeof(int));
			    if(status)
			        printf("Server directory successfully changed\n");
			    else
			        printf("Server directory failed to change\n");
			    break;

			case 5:
			  	strcpy(buf, "pwd");
			  	write(sockfd, buf, 100);
			  	read(sockfd, buf, 100);
			  	printf("%s\n", buf);
			  	break;

			case 6:
				system("ls");
				break;

			case 7:
				bzero(buf,100);
				scanf("%s",buf);
				if(chdir(buf) < 0)
					printf("clicd: No such file or directory\n");
			case 8:
				system("pwd");
				break;

			case 9:
			  	strcpy(buf, "quit");
			    write(sockfd, buf, 100);
			    read(sockfd, &status, 100);
			  	if(status)
			    {
			      	printf("Server closed\nQuitting..\n");
			      	exit(0);
			    }
			    printf("Server failed to close connection\n");

			default :
				printf("An invalid ftp command.\n");
		}
	}
	close(datasockfd);
	close(sockfd);
}
