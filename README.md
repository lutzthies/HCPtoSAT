HCP to SAT (or as we call it: Helga)
===
 Hey there! :v:

This repo contains public versions of the encoder Helga, which translates [Hamiltonian Cycle Problems](https://en.wikipedia.org/wiki/Hamiltonian_path_problem) for given graphs into [SAT problems](https://en.wikipedia.org/wiki/Boolean_satisfiability_problem). Input and output data is represented in the commonly known [DIMACS format](http://www.satcompetition.org/2009/format-benchmarks2009.html).

About
---
The project was developed during a competition at the Technische Universität Dresden in the summer term of 2016. Our encoder *helga_multiPB* in combination with the solver [RSAT 3.01](http://reasoning.cs.ucla.edu/rsat/index.html) won the contest. We were able to achieve an advance in solving time of about 22.000 percent (in case you're wondering, that ain't a decimal mark, this does in fact read twenty two thousand) respective to the 2nd place. :tada:

This was possible thanks to a new, so-called "relative" encoding. But if you take a closer look, you'll find out that we actually do not solve the Hamiltonian Cycle Problem. Instead we make use of a mathematical trick to transform the HCP into the Hamiltonian Path Problem and then solve the HPP itself. Furthermore there is a lot of constraint and clause optimization going on. If you're interested in the theory behind all of this or if any other questions arise, feel free to get in touch with us.

At this point we'd like to get out special thanks to Peter Steinke. He developed the library [PBLib](http://tools.computational-logic.org/content/pblib.php) that we slightly modified and now use exhaustively throughout the encoder for parts of said optimizations. In addition to that, he also did a great job supporting us and answering all our annoying questions. Thank you so much. :thumbsup:

Note, that the code may be quite bad and/or contain undefined behavior at certains points. Please have patience with us, as we programmed with a main focus on correct results based on correctly provided input by us and not on error handling. Time will tell, if we continue to work on this. So for now there's nothing left to say than:

Happy segfaults and experimenting,
Cheers! :beers:

Structure
---
The main folder of this repository contains the following sub-folders:
* *encoders*

  We distinguish between single- and multithreaded versions of our encoder by using the suffix **sequential** and **multi**. If PBLib is used for clause optimization **PB** is appended to the name in addition to the type.

  Example:
  ```
  ./encoder/helga_multiPB
  ```
  The version that won features multithreading and uses PBLib.

* *solvers*

  You can use the encoders independently with different SAT solvers without touching any code at all. Simply put a sub-folder with your desired solver in this directory and refer to the instructions on how to use it.

  Example:
  ```
  ./solvers/lingeling/plingeling
  ```
  We included [lingeling](http://fmv.jku.at/lingeling/) in this repository. It comes with a massively parallels version called *plingeling*.

* *graphs*

  To keep things tidy, especially when testing hundreds of graphs, we put all of them in one place.

  Example:
  ```
  ./graphs/hc-4.col
  ```
  Oh, by the way, for starters we left you that very simple graph in there. :blush:

* *pblib*

  This folder contains the modified version of PBLib, usually you should not have anything to do with this. Except building it once while setting up the environment.

First Steps
---
1. Head over to the [releases](LINK) page and grab the latest one. Alternatively you can of course (fork and) clone this repository.

2. Build the modified version of PBLib by changing into the *pblib* directory and running `cmake .` followed by `make`.

3. Switch to *solvers/lingeling* and build it by running `make`(without the preceding `cmake .` command). If you want to use other SAT solvers, for instance the one we used in the competition, check out the [Instructions](LINK).

4. Finally, pick an encoder from the *encoders* folder and build it by repeating the steps in (2).

  You can then start the encoding and solving process by executing a command of the following structure from within the encoders directory (this is crucial):

  ```
./encoders/helga_[type][PB]/helga.out [graph] [command for the solver]
```
Example:
```
./encoders/helga_multiPB/helga.out ../../graphs/hc-4.col "../../solvers/lingeling/plingeling in.txt"
```

This is it. After some time (this can reach from milliseconds to minutes, dependent on the size of the graph) you will be greeted with either

```
s SATISFIABLE
v 1 4 3 2
```
which means, that the path 1->4->3->2->1 is a Hamiltonian Cycle (HC) in graph, or
```
s UNSATISFIABLE
```
which means, that no HC has been found (and we are quite certain, there indeed is none).

Instructions
------

* Use other SAT solvers

Due to licensing issues only *lingeling* is directly provided in this repo. However, we determined that best results in solving time can be achieved with RSAT. Simply head over to [their page](http://reasoning.cs.ucla.edu/rsat/download_new.php), sign up and download RSAT. Then put it into this directory and use the following solver command:
```
bash ./rsat.sh in.txt -s
```
Other solvers that we tested and found to be compatible with our encoders are *RISS* and *miniSAT*. But generally speaking all solvers that support input from and output to text files in the standard DIMACS format should produce somewhat usable results.

License
---
If not explicitly stated otherwise the code in this repository is released under:

>The MIT License (MIT)

>Copyright (c) 2016 Lukas Klose, Josia Mädler and Lutz Thies

The full full license text is provided in the *[LICENSE](LINK)* file.
