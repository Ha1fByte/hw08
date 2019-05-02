Names: Natalie Cardinali and Sean Francis
Project Choice : Copy Garbage Collector for microScheme
Time spent on assignment : 6hrs
Additional Collaborators:

... description of your solution to the problem ...
Using our knowlege of how the copy gargbage collector works we created a copy 
function and added to the collect function. 

The copy function is called from the
collect function, and it moves values from the fromspace to the tospace and then 
swaps the pointers to each space.

The collect function is run when allocloc finds that the heap pointer is equal 
to the heaplimit. It calls collect which first checks if the heap is initalized.  
If not, it initializes the space then continues on, copying the roots and then 
having the scanloc pointer itterate through all of the copied values to see if 
they have any pointers pointing outside the tospace. If so, those values are 
forwarded over. This continues unitl the scanloc pointer meets up with the heap 
pointer. 

After all the values are copied and the spaces are swapped the collect function 
checks to make sure that the heap pointer isn't still equal to the heaplimit. If 
that's the case, that means that all the object on the heap are live and that 
the heap is too small to hold them all. The collect function then increases the 
size of the tospace, then copies all the values in the from spcae over like it 
did before. After the values are in the new, larger tospace, the program 
increases the size of smaller fromspace so it's the same size as the to space. 

Then it swaps the pointer like in normal execution and returns to allocloc. 


... description of how you tested your submission ...
To test the program we used the evaltest.scm file provided. By running the 
following command: 

    cat eval.scm evaltest.scm | ./uscheme-copy -q > ourOutput.out

Then we used `diff ourOutput.out eval_evaltest.out`. To check the functionality
and print statements of our garbage collector. Once the outputs matched we knew 
the garbace collector worked. Just as an extra step to verify, we ran prog03.scm 
and prog03_tests.scm to make sure we didn't have any errors that could've been 
missed by the tests provided. 

... description of your submission 's functionality ...
After testing with the two different programs, we are confident in saying that
collect(), copy(), and printfinalstats() all work as intended. Overall this
means our garbage collector succesfully implements the Copy GC for microScheme. 

