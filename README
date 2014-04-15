* What is in this directory? 

  README           this file
  chatclient.c     contains source code of chat client
  chatserver.c     contains source code of chat server
  chatlinker.c     routines used by server and client
  common.h         header file included by all .c files
  makefile         make file to build chatserver and chatclient

  goodchatserver   chat server executable 
  goodchatclient   chat client executable
  groups	   example groups file

* How do I compile my programs? 

  The 'makefile' is provided just for that purpose. To compile, run

    % make

  It will create 'chatserver' and 'chatclient'.

* How do I run these programs? 

  The 'chatserver' program takes  a command line argument <groups-file>.
  For example you can run it as follows.

    % chatserver groups
 
  The 'chatclient' program doesn't take any arguments and you simply run
  it as

    % chatclient

  You run the server first and then many clients. The clients connect to
  the server and prompt you to enter a group name. Here is an example 

    admin: connected to server on 'zeus.ite.gmu.edu' at '35739'
	      group        capacity       occupancy
	    singles               5               0
	    vikings               7               0
	   politics               2               0
	    cricket               9               0
    which group? 

  If you  enter group name, it  will prompt you for  your nickname. Once
  you enter that it will look as follows 

    admin: connected to server on 'zeus.ite.gmu.edu' at '35739'
	      group        capacity       occupancy
	    singles               5               0
	    vikings               7               0
	   politics               2               0
	    cricket               9               0
    which group? singles
    what nickname? gary
    admin: joined 'singles' as 'gary'

  If other clients are also run and become members, whatever is typed by
  a client would appear at all clients of that particular group. 

* How do I exit from these programs? 

  In  case of  client, if  you  type "/end"  or press  'Return' when  it
  prompts you for group name or nickname, it will exit. 

  If  the client  already joined  a group,  typing "/end'  on a  line by
  itself will make  the client leave the group. It  will once again list
  the groups and prompt you for group name.

  At this point you can enter  some other group or exit by typing "/end"
  or pressing 'Return' as explained before. 

  In case of server, you can just press Ctrl-C to kill it. 

* Do these programs run on any machine? 

  They are compiled on thor.vse.gmu.edu. They may work on other
  machines  with different  versions.

* I am getting the following errors. What do i do?

  When you run 'chatserver' if it says

    error : server already exists

  that means  either a  'chatserver' is already  running or  somehow the
  previous 'chatserver' didn't exit cleanly. In the second case, you can
  do the cleanup manually by removing the '.chatport'. You can check and
  delete the link by doing as follows.

    % cd
    % ls -l .chatport
    .chatport -> zeus.ite.gmu.edu:35743
    % rm .chatport


  When you run 'chatclient' if it says
    
    error : no active chat server

  it just means  that 'chatserver' is not running.  Run the 'chatserver'
  before you run the clients. 
