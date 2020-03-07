//
// Created by Agnieszka on 07/03/2020.
//

#include <stdlib.h>
#include "library.h"

int main(int argc, char **argv){
//    system("cd /mnt/d/Agnieszka/Documents/Studia/4semestr/SO/lab1 && diff a.txt b.txt > c.txt");
    char string[50] = "a.txt:b.txt c.txt:d.txt a.txt:c.txt";
    int nr_of_pairs=3;
    compare_pairs(string, nr_of_pairs);
    return 0;
}

