File: readme.txt
Assignment: assign0
Author: Jie Zhu
Pronouns: Jie
----------------------

0. Tell us about yourself!  What do you do for fun?  Tell a quick anecdote about something that you feel makes you unique; a talent, experience, anything.
   I usually play violin for fun. 


1. Please initial below to confirm your course enrollment.

[ JZ] I will attend the CS111 final as scheduled and I understand no alternate/makeup final exams will be offered except for OAE or official athletics accommodations.  The final exam is in person and the time is listed on the course website, cs111.stanford.edu.
[JZ ] I have completed the Honor Code quiz on the assign0 page, and I have read and understood the CS111 collaboration policy at https://cs111.stanford.edu/collaboration.html. By adding my initials, I affirm my intention to abide by this policy.
[ JZ] If applicable, I have contacted the course staff about any unmoveable academic/University conflicts with the midterm exam time.  The midterm exam time is listed on the course website, cs111.stanford.edu.

Please remember to send us your OAE letter (if applicable) as soon as possible!  Thank you.


Question 2: which line in the file causes the crash?  In 1-2 sentences, explain how you used GDB to find the answer, including which command(s) you used.  Use GDB commands as much as possible, even if you are able to answer the question without using GDB.
---- Line 37 (strcpy) in the file causes the crash. I use command "run hello cs111!" in gdb, and use backtrace to see what causes the segment fault. last doesn't have allocated memory to copy.


Question 3: what is the value of `i` in the loop in `main` when the program crashes?  In 1-2 sentences, explain how you used GDB to find the answer, including which command(s) you used.  Use GDB commands as much as possible, even if you are able to answer the question without using GDB.
---- The value of i is 2. I use command "run hello cs111!" in gdb, and use backtrace to see what causes the segment fault. Then I use command frame 2 to get into main function,
     and type print i to see what is the value of i before crash.


Question 4: if we implemented the destructor for `Movie` to print out a message "Movie X is being destroyed" (where X is replaced with the movie name), and we ran test 1, when would this message be printed?
---- This message will be printed at the end after print statement in either if or else statement


