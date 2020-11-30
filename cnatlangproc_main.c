#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// User-defined macros
#define TOKEN_LENGTH 64
#define STR_MAX_LENGTH 128

// User-defined data types
typedef struct {
    char succ_token[TOKEN_LENGTH];
    int hits;
    int hit_low_bound;
    int hit_hi_bound;
} succ_token_stat, *succ_token_stat_ptr;

typedef struct input_token {
    char token[TOKEN_LENGTH];
    succ_token_stat_ptr succ_token_list;
    int succ_token_list_size;
    struct input_token *next;
} curr_token, *curr_token_ptr;

typedef struct {
    char key_token[TOKEN_LENGTH];
    curr_token_ptr token_location;
} token_hash_map, *token_hash_map_ptr;

typedef struct {
    token_hash_map_ptr token_hash;
    int hash_size;
} token_hash_map_pkg, *token_hash_map_pkg_ptr;

// Function Prototypes
int get_tokens(int, curr_token_ptr *, token_hash_map_pkg_ptr *);

inline int proc_subseq_tokens(char *, curr_token_ptr *, token_hash_map_pkg_ptr *);

int comp_succ_token(void const *, void const *);
int comp_succ_token_hits(void const *, void const *);
int comp_token_hash(void const *, void const *);

void disp_hash_map(token_hash_map_pkg_ptr);
void disp_prob_dist(curr_token_ptr);

void proc_prob_dist(curr_token_ptr);
int gen_string_from_prob_dist(curr_token_ptr, token_hash_map_pkg_ptr, int);

void clean_up_token_list(curr_token_ptr *);

// Main Function
// This also contains the main user interface for the program
int main (void) {
    // Data Handles
    int choice = 0, string_size = 0, err_flag = 0;
    token_hash_map_pkg_ptr hashmap = NULL;
    curr_token_ptr head = NULL;

    printf("C Exercise: Natural Language Processing\n");

    // Start of the program

    if (!err_flag) {
        while ((choice != 3) && (!err_flag)) {
            printf("\nChoose from the following:\n");
            printf("(1) Enter text manually\n");
            printf("(2) Acquire text from file\n");
            printf("(3) Exit\n");

            printf("\nEnter choice (1, 2 or 3): ");

            scanf("%d", &choice);

            // http://stackoverflow.com/questions/4929338/problem-with-scanf-and-fgets
            fflush(stdin);

            switch (choice) {
                case 1 : {
                    printf("\nEnter text manually...\n");
                    err_flag = get_tokens(choice, &head, &hashmap);
                    break;
                }
                case 2 : {
                    printf("\nAcquire text from file...\n");
                    err_flag = get_tokens(choice, &head, &hashmap);
                    break;
                }
                case 3 : {
                    printf("\nExiting...\n");
                    break;
                }
                default : {
                    printf("Invalid choice! Kindly enter a valid choice.\n");
                    break;
                }
            }

            if (((choice == 1) || (choice == 2)) && (!err_flag)) {

                proc_prob_dist(head);

                printf("\nInput has been processed...\n");

                while ((choice != 3) && (!err_flag)) {

                    printf("\nChoose from the following:\n");
                    printf("(1) Print probability distribution\n");
                    printf("(2) Generate string from probability distribution\n");
                    printf("(3) Exit\n");

                    printf("\nEnter choice (1, 2 or 3): ");

                    scanf("%d", &choice);

                    // http://stackoverflow.com/questions/4929338/problem-with-scanf-and-fgets
                    fflush(stdin);

                    switch (choice) {
                        case 1 : {
                            disp_prob_dist(head);
                            break;
                        }

                        case 2 : {
                            printf("\nGenerating string...\n");
                            do {
                                printf("\nEnter desired no. of words: ");
                                scanf("%d", &string_size);

                                // http://stackoverflow.com/questions/4929338/problem-with-scanf-and-fgets
                                fflush(stdin);
                                if (string_size > 0) {
                                    err_flag = gen_string_from_prob_dist(head, hashmap, string_size);
                                } else {
                                    printf("\nInvalid entry, kindly enter again ...");
                                }
                            } while (string_size < 0);
                            break;
                        }

                        case 3 : {
                            printf("\nExiting...\n");
                            break;
                        }

                        default : {
                            printf("Invalid choice! Kindly enter a valid choice.\n");
                            break;
                        }
                    }
                } // while (choice != 3) {
            } // if ((choice == 1) || (choice == 2)) {
        } // while(choice != 3) {
    } // if (!err_flag) {

    // Clean-up
    clean_up_token_list(&head);

    if (hashmap != NULL) {
        if (hashmap->token_hash != NULL) {
            free(hashmap->token_hash);
            hashmap->token_hash = NULL;
        }
        free(hashmap);
        hashmap = NULL;
    }

    return err_flag;
}

// Function     : get_tokens()
// Description  : This function acquires token from the screen and fills up
//                the token list and hash map accordingly.
// Parameters   : tokens  - token list pointer
//                map_pkg - hash map package pointer
// Return Value : error status (0 - no error, 1 - error)
int get_tokens(int choice, curr_token_ptr *tokens, token_hash_map_pkg_ptr *map_pkg) {
    char *buff = NULL, *temp_buff = NULL, *buffer1 = NULL, buffer2[STR_MAX_LENGTH], buffer3[TOKEN_LENGTH], buffer4[TOKEN_LENGTH];
    FILE *in_handle = NULL;
	int temp_length = 0, err_flag = 0, total_tokens = 0;

    // Temporary Handles
    succ_token_stat_ptr temp_succ_token = NULL;
    curr_token_ptr temp_tokens = NULL;
	
	if (choice == 1) {
		in_handle = stdin;
		printf("\nEnter text (Hit ENTER when finished):\n");
	} else if (choice == 2) {
		printf("\nEnter file name (Hit ENTER when finished): ");
		scanf("%s", buffer2);
		fflush(stdin);
		in_handle = fopen(buffer2, "r");
		
		if (in_handle == NULL) {
			printf("File %s can't be opened\n", buffer2);
			err_flag = 1;
		} else {
			printf("File %s now being tokenized ...\n", buffer2);
		}
	} else {
		printf("Error in choice\n");
        err_flag = 1;
	}

    if (!err_flag) {
        // Initialize Hash Map Package
        if (((*map_pkg) = (token_hash_map_pkg_ptr)malloc(sizeof(token_hash_map_pkg))) == NULL) {
            printf("Error in initializing the hash map package\n");
            err_flag = 1;
        } else {
            if (((*map_pkg)->token_hash = (token_hash_map_ptr)malloc(sizeof(token_hash_map))) == NULL) {
                printf("Error in initializing the hash map\n");
                err_flag = 1;
            } else {
                (*map_pkg)->hash_size = 0;
            }
        } // if ((hashmap ...

        if (!err_flag) {
            if (((*tokens) = (curr_token_ptr)malloc(sizeof(curr_token))) == NULL) {
                printf("Error in initializing token list\n");
                err_flag = 1;
            } else {
                (*tokens)->succ_token_list = NULL;
                (*tokens)->succ_token_list_size = 0;
                (*tokens)->next = NULL;
            }
        } // if (!err_flag)

        if (!err_flag) {
            // Initialize succeeding token list
            if ((temp_succ_token = (succ_token_stat_ptr)malloc(sizeof(succ_token_stat))) == NULL) {
                printf("Error in initializing succeeding token list\n");
                err_flag = 1;
            } else {
                temp_succ_token->hits = 0;
                (*tokens)->succ_token_list = temp_succ_token;
                temp_tokens = *tokens;
            } // if ((temp_succ_token = ...
        }

        if (!err_flag) {
            // Start generating token list from temporary handle
            
            // If - statement first since this is just the first word
            if (fscanf(in_handle, "%s", buffer3) == 1) {
                // place 1st word
                strcpy(temp_tokens->token, buffer3);

                // also, place the word in the hash map
                strcpy((*map_pkg)->token_hash[(*map_pkg)->hash_size].key_token, buffer3);
                (*map_pkg)->token_hash[(*map_pkg)->hash_size].token_location = temp_tokens;
                (*map_pkg)->hash_size = 1;

                total_tokens++;
                			
				do {
					if ((buffer1 = (char*)malloc(sizeof(char))) == NULL) {
						printf("Error in allocating buffer\n");
						err_flag = 1;
					} else {
						buffer1[0] = '\0';
					} // if ((buffer1 = ... 
					
					//Get initial line
					fgets(buffer2, STR_MAX_LENGTH, in_handle);
					
					buffer1 = (char*)realloc(buffer1, (((strlen(buffer2))+temp_length+1)*sizeof(char)));

					if (buffer1 == NULL) {
						printf("Error in allocating buffer\n");
						err_flag = 1;
						break;
					}									

					buffer1[temp_length] = '\0';

					strcat(buffer1,buffer2);
					temp_length = strlen(buffer1);
					
					//Check if fgets cuts in a middle of a token
					if ((strlen(buffer2) == STR_MAX_LENGTH-1) && (buffer2[strlen(buffer2)-1] != ' ')) {
						fscanf(in_handle, "%s", buffer3);
						buffer1 = (char*)realloc(buffer1, (((strlen(buffer3))+temp_length+1)*sizeof(char)));
							
						if (buffer1 == NULL) {
							printf("Error in allocating buffer\n");
							err_flag = 1;
							break;
						}
							
						buffer1[temp_length] = '\0';
						strcat(buffer1,buffer3);
					}
						
					buffer1[strlen(buffer1)-1] = '\0';
					
					temp_buff = buffer1;
										
					// get subsequent words
					buff = strtok(temp_buff, " ");
					while ((!err_flag) && (buff != NULL)) {
						err_flag = proc_subseq_tokens(buff, &temp_tokens, map_pkg);

						if (!err_flag) {
							total_tokens++;
						}
						
						buff = strtok(NULL, " ");
					} // while ((!err_flag) && (buff != NULL)) {

					if (buffer1 != NULL) {
						free(buffer1);
						buffer1 = NULL;
					}
					
				} while (((choice == 1) && (!((strlen(buffer2) < STR_MAX_LENGTH-1 ) || (buffer2[strlen(buffer2)-1]== '\n') ))) || 
					((choice == 2) && (!feof(in_handle))));
            } else {
                printf("Error ... no valid token\n");
                err_flag = 1;
            } //if (fscanf(in_handle, "%s", buffer3) == 1) {
        } // if (!err_flag) {
    } // if (!err_flag) {
    // Free up the temporary handle

	if ((in_handle != NULL) && (choice == 2)) {
        fclose(in_handle);
    }

    if (!err_flag) {
        printf("\nProcessed a total of %d tokens.\n", total_tokens);
    }

    return err_flag;
}

// Function     : proc_subseq_tokens()
// Description  : This function processes the subsequent tokens from screen or file.
// Parameters   : buffer - token to be processed
//                tokens  - token list pointer
//                map_pkg - hash map package pointer
// Return Value : error status (0 - no error, 1 - error)
inline int proc_subseq_tokens(char *buffer, curr_token_ptr *tokens, token_hash_map_pkg_ptr *map_pkg) {
    succ_token_stat_ptr temp_succ_token_key = NULL, temp_succ_token = NULL;
    token_hash_map_ptr temp_hash_key = NULL, temp_hash = NULL;
    int err_flag = 0;

    // Perform binary search
    // Create a temporary key

    temp_succ_token_key = (succ_token_stat_ptr) malloc(sizeof(succ_token_stat));

    if (temp_succ_token_key == NULL) {
        printf("Error in creating the succeeding token key\n");
        err_flag = 1;
    }

    if (!err_flag) {
        strcpy(temp_succ_token_key->succ_token, buffer);

        temp_succ_token = bsearch(temp_succ_token_key, (*tokens)->succ_token_list,
                                  (*tokens)->succ_token_list_size, sizeof(succ_token_stat),
                                  comp_succ_token);

        if (temp_succ_token == NULL) {
            // Reallocate
            if ((*tokens)->succ_token_list_size != 0) {
                (*tokens)->succ_token_list = (succ_token_stat_ptr)realloc((*tokens)->succ_token_list,
                                                                            (((*tokens)->succ_token_list_size)+1) *
                                                                            sizeof(succ_token_stat));

                if ((*tokens)->succ_token_list == NULL) {
                    printf("Error in reallocating succeeding token list\n");
                    err_flag = 1;
                }
            } // ((*tokens)->succ_token_list_size != 0) {

            if (!err_flag) {
                strcpy((*tokens)->succ_token_list[(*tokens)->succ_token_list_size].succ_token, buffer);
                (*tokens)->succ_token_list[(*tokens)->succ_token_list_size].hits = 1;

                (*tokens)->succ_token_list_size++;

                // Perform quicksort
                qsort((*tokens)->succ_token_list, (*tokens)->succ_token_list_size,
                      sizeof(succ_token_stat), comp_succ_token);
            }
        } else {
            temp_succ_token->hits += 1;
        }

        free(temp_succ_token_key);
        temp_succ_token_key = NULL;

        // Add to hash map
        // Binary search
        // Create temporary key

        temp_hash_key = (token_hash_map_ptr) malloc(sizeof(token_hash_map));

        if (temp_hash_key == NULL) {
            printf("Error in creating the hash map key\n");
            err_flag = 1;
        }

        if (!err_flag) {
            strcpy(temp_hash_key->key_token, buffer);

            temp_hash = bsearch(temp_hash_key, (*map_pkg)->token_hash, (*map_pkg)->hash_size,
                                sizeof(token_hash_map), comp_token_hash);

            if (temp_hash == NULL) {
                // Reallocate

                //printf("\nAdding %s to hash map\n", buffer);

                (*map_pkg)->token_hash = (token_hash_map_ptr)realloc((*map_pkg)->token_hash,
                                                                 ((*map_pkg)->hash_size+1) *
                                                                 sizeof(token_hash_map));

                if ((*map_pkg)->token_hash  == NULL) {
                    printf("Error in reallocating hash map\n");
                    err_flag = 1;
                }

                if (!err_flag) {
                    strcpy((*map_pkg)->token_hash[(*map_pkg)->hash_size].key_token, buffer);

                    // Since it is not found in the hash map, then this would also be added in the
                    // token list

                    temp_succ_token = (succ_token_stat_ptr) malloc(sizeof(succ_token_stat));
                    if (temp_succ_token == NULL) {
                        printf("Error in initializing succeeding token list\n");
                        err_flag = 1;
                    }

                    if (!err_flag) {

                        while ((*tokens)->next != NULL) {
                            (*tokens) = (*tokens)->next;
                        }

                        (*tokens)->next = (curr_token_ptr)malloc(sizeof(curr_token));

                        if ((*tokens)->next == NULL) {
                            printf("Error in initializing token list\n");
                            err_flag = 1;
                        }

                        if (!err_flag) {
                            (*tokens) = (*tokens)->next;
                            (*tokens)->succ_token_list = NULL;
                            (*tokens)->succ_token_list = temp_succ_token;
                            (*tokens)->succ_token_list_size = 0;
                            (*tokens)->next = NULL;

                            strcpy((*tokens)->token, buffer);
                            (*map_pkg)->token_hash[(*map_pkg)->hash_size].token_location = (*tokens);
                            (*map_pkg)->hash_size++;

                            // Perform quicksort
                            qsort((*map_pkg)->token_hash, (*map_pkg)->hash_size,
                                  sizeof(token_hash_map), comp_token_hash);
                        }
                    }
                }
            } else {
                (*tokens) = temp_hash->token_location;
            }

            free(temp_hash_key);
            temp_hash_key = NULL;
        }
    }

    if (temp_succ_token_key != NULL) {
        free(temp_succ_token_key);
        temp_succ_token_key = NULL;
    }

    if (temp_hash_key != NULL) {
        free(temp_hash_key);
        temp_hash_key = NULL;
    }

    return err_flag;
}

// Function     : comp_succ_token()
// Description  : This function compares 2 elements in a succeeding token list.
//                It does a string comparison on the succeeding tokens.
//                This is used as a parameter to bsearch() and qsort() to be able
//                to process and generate a token list
// Parameters   : lhs - pointer being compared
//                rhs - pointer being compared with
// Return Value : comparison result
//                (-) value - lhs is less than rhs
//                (+) value - lhs is greater than rhs
//                 0        - lhs and rhs are equal
int comp_succ_token(void const *lhs, void const *rhs) {
    succ_token_stat const *const token_a = lhs;
    succ_token_stat const *const token_b = rhs;

    return strcmp(token_a->succ_token, token_b->succ_token);
}

// Function     : comp_succ_token_hits()
// Description  : This function compares 2 pointers in the succeeding token list.
//                However, different from the comp_succ_token() function, this compares
//                the generated hit against the one found in the hash map.
//                This is used as a parameter in bsearch() in determining the next token
//                generated using the probability distribution from the input text.
// Parameters   : lhs - pointer being compared
//                rhs - pointer being compared with
// Return Value : comparison result
//                (-) value - generated hit is less than the boundary of token
//                (+) value - generated hit is greater than the boundary of token
//                 0        - generated hit is within the boundaries of token
int comp_succ_token_hits(void const *lhs, void const *rhs) {
    succ_token_stat const *const token_a = lhs;
    succ_token_stat const *const token_b = rhs;

    if ((token_a->hit_low_bound >= token_b->hit_low_bound) && (token_a->hit_low_bound <= token_b->hit_hi_bound)) {
        return 0;
    } else {
        if (token_a->hit_low_bound < token_b->hit_low_bound) {
            return -1;
        } else {
            // hit would obviously be greater than the higher bound hit
            return 1;
        }
    }
}

// Function     : comp_token_hash()
// Description  : This function compares 2 elements in the hash map.
//                It does a string comparison on the tokens.
//                This is used as a parameter to bsearch() and qsort() to be able
//                to process and generate a token list
// Parameters   : lhs - pointer being compared
//                rhs - pointer being compared with
// Return Value : comparison result
//                (-) value - lhs is less than rhs
//                (+) value - lhs is greater than rhs
//                 0        - lhs and rhs are equal
int comp_token_hash(void const *lhs, void const *rhs) {
    token_hash_map const *const hash_a = lhs;
    token_hash_map const *const hash_b = rhs;

    return strcmp(hash_a->key_token, hash_b->key_token);
}

// Function     : disp_hash_map()
// Description  : This function displays the hash map.  This is mainly for debug purposes.
// Parameters   : hash_map_pkg - Hash Map Package
// Return Value : None
void disp_hash_map(token_hash_map_pkg_ptr hash_map_pkg) {
    int index;

    printf("\nPrinting the Hash Map ...\n");
    printf("\nHash Map Size: %d\n", hash_map_pkg->hash_size);

    for (index = 0; index < hash_map_pkg->hash_size; index++) {
        printf("Hash Index %d Token: %s\n", index+1, hash_map_pkg->token_hash[index].key_token);
        printf("Hash Index %d Address->token: %s\n", index+1, hash_map_pkg->token_hash[index].token_location->token);
    }
}

// Function     : disp_prob_dist()
// Description  : This function displays the acquired token and its succeeding tokens
//                from the input text. For the succeeding tokens, it displays the total number
//                of hits/occurrences of the token and the probability it is the succeeding token.
// Parameters   : token_list - generated token list
// Return Value : None
void disp_prob_dist(curr_token_ptr token_list) {
    curr_token_ptr temp_list = NULL;
    char file_name[TOKEN_LENGTH];
    int index = 0, temp_max = 0, choice = 0;
    FILE *out_handle = NULL;

    temp_list = token_list;

    printf("\nPrinting Hits and Probability Distributions ...\n");

    do {
        printf("\nChoose from the following:\n");
        printf("(1) Print in screen\n");
        printf("(2) Print to CSV file\n");

        printf("\nEnter choice (1 or 2): ");
        scanf("%d", &choice);
        fflush(stdin);

        if (choice == 2) {
            printf("\nEnter file name (Don't place any extension as it would be .csv): ");

            scanf("%s", file_name);
            fflush(stdin);

            strcat(file_name,".csv");

            out_handle = fopen(file_name, "w");
            if (out_handle == NULL) {
                printf("File %s can't be opened\n", file_name);
                printf("Input another file name or select to output to screen\n");
            } else {
                printf("File %s now being written to (previous content destroyed) ...\n", file_name);
            }
        }

        if ((choice != 1) && (choice != 2)) {
            printf("Invalid choice! Kindly enter a valid choice.\n");
        }

    } while (((choice != 1) && (choice != 2)) || ((choice == 2) && (out_handle == NULL)));

    if (choice == 1) {
        while (temp_list != NULL) {
            printf("\nToken: %s\n", temp_list->token);

            if (temp_list->succ_token_list != NULL) {
                temp_max = temp_list->succ_token_list[temp_list->succ_token_list_size-1].hit_hi_bound + 1;

                printf("\n%s has a total of %d succeeding tokens.\n", temp_list->token, temp_list->succ_token_list_size);

                for(index = 0; index < temp_list->succ_token_list_size; index++) {
                    printf("%s was followed by %20s %10d time(s) (%5.2f%%)\n",
                        temp_list->token, temp_list->succ_token_list[index].succ_token,
                        temp_list->succ_token_list[index].hits,
                        (float)temp_list->succ_token_list[index].hits*100/temp_max);

                }
            } else {
                printf("%s has no succeeding tokens\n", temp_list->token);
            }
            printf("\nPress ENTER to continue ...\n");
            getchar();
            fflush(stdin);
            temp_list = temp_list->next;
        }
    } else {
        fprintf(out_handle, "\"Token\",\"Next Token\",\"Hits\",\"Percentage\"\n");
        while (temp_list != NULL) {
            fprintf(out_handle, "\"%s\"", temp_list->token);

            if (temp_list->succ_token_list != NULL) {
                temp_max = temp_list->succ_token_list[temp_list->succ_token_list_size-1].hit_hi_bound + 1;

                for(index = 0; index < temp_list->succ_token_list_size; index++) {
                    fprintf(out_handle, ",\"%s\",\"%d\",\"%.2f%%\"\n",
                        temp_list->succ_token_list[index].succ_token,
                        temp_list->succ_token_list[index].hits,
                        (float)temp_list->succ_token_list[index].hits*100/temp_max);

                }
            } else {
                fprintf(out_handle, ",,,\n");
            }
            temp_list = temp_list->next;
        }
        fclose(out_handle);
    }
}

// Function     : proc_prob_dist()
// Description  : This function processes probability distributions for each element
//                in the token list. It produces the hit boundaries for each succeeding
//                token to be used in token generation later.
// Parameters   : token_list - token list
// Return Value : None
void proc_prob_dist(curr_token_ptr token_list) {
    curr_token_ptr temp_list = NULL;
    int index = 0, temp_bound = 0;

    temp_list = token_list;

    printf("\nProcessing Probability Distributions ...\n");

    while (temp_list != NULL) {
        if ((temp_list->succ_token_list != NULL) && (temp_list->succ_token_list_size == 0)) {
            free(temp_list->succ_token_list);
            temp_list->succ_token_list = NULL;
        }

        if (temp_list->succ_token_list != NULL) {
            temp_bound = 0;
            for(index = 0; index < temp_list->succ_token_list_size; index++) {
                temp_list->succ_token_list[index].hit_low_bound = temp_bound;
                temp_list->succ_token_list[index].hit_hi_bound = temp_bound + temp_list->succ_token_list[index].hits - 1;
                temp_bound = temp_bound + temp_list->succ_token_list[index].hits;
            }
        }
        temp_list = temp_list->next;
    }
}

// Function     : gen_string_from_prob_dist()
// Description  : This function generates an output text from a token list processed from an earlier
//                input text. It also uses a hash map to get to the next token along the list.
//                It generates an output text with a specific size (input).
// Parameters   : token_list - token list processed from input text
//                map_pkg - hash map package
//                string_size - no. of tokens of output text
// Return Value : error status (0 - no error, 1 - error)
int gen_string_from_prob_dist(curr_token_ptr token_list, token_hash_map_pkg_ptr map_pkg, int string_size) {
    int index = 0, gen_hit = 0, err_flag = 0, choice = 0;
    curr_token_ptr temp_list = NULL;
    succ_token_stat_ptr temp_succ_token_key = NULL, temp_succ_token = NULL;
    token_hash_map_ptr temp_hash, temp_hash_key = NULL;
    FILE *out_handle = NULL;
    char file_name[TOKEN_LENGTH];

    do {
        printf("\nChoose from the following:\n");
        printf("(1) Print in screen\n");
        printf("(2) Print to text file\n");

        printf("\nEnter choice (1 or 2): ");
        scanf("%d", &choice);
        fflush(stdin);

        if (choice == 2) {
            printf("\nEnter file name (Don't place any extension as it would be .txt): ");

            scanf("%s", file_name);
            fflush(stdin);

            strcat(file_name,".txt");

            out_handle = fopen(file_name, "w");
            if (out_handle == NULL) {
                printf("File %s can't be opened\n", file_name);
                printf("Input another file name or select to output to screen\n");
            } else {
                printf("File %s now being written to (previous content destroyed) ...\n", file_name);
            }
        }

        if ((choice != 1) && (choice != 2)) {
            printf("Invalid choice! Kindly enter a valid choice.\n");
        }
    } while (((choice != 1) && (choice != 2)) || ((choice == 2) && (out_handle == NULL)));

    srand(time(0));

    temp_list = token_list;

    printf("\nGenerating string with %d words...\n\n", string_size);

    // Print first word
    if (choice == 1) {
        printf("%s", temp_list->token);
    } else {
        fprintf(out_handle, "%s", temp_list->token);
    }

    // Print subsequent words
    for (index = 0; index < string_size-1; index++) {
        if (temp_list->succ_token_list_size == 0) {
            printf("\n\nCan no longer generate token since %s is the last word of the input text\n", temp_list->token);
            break;
        }
        gen_hit = rand() *(temp_list->succ_token_list[temp_list->succ_token_list_size-1].hit_hi_bound);

        gen_hit = gen_hit / RAND_MAX;

        // Perform binary search
        // Create a temporary key

        temp_succ_token_key = (succ_token_stat_ptr) malloc(sizeof(succ_token_stat));

        if (temp_succ_token_key == NULL) {
            printf("Error in creating the succeeding token key\n");
            err_flag = 1;
            break;
        }

        temp_succ_token_key->hit_low_bound = gen_hit;

        temp_succ_token = bsearch(temp_succ_token_key, temp_list->succ_token_list,
                                  temp_list->succ_token_list_size, sizeof(succ_token_stat),
                                  comp_succ_token_hits);

        if (temp_succ_token != NULL) {
            // Binary search
            // Create temporary key
            temp_hash_key = (token_hash_map_ptr) malloc(sizeof(token_hash_map));
            if (temp_hash_key == NULL) {
                printf("Error in creating the hash map key\n");
                err_flag = 1;
                break;
            }

            strcpy(temp_hash_key->key_token, temp_succ_token->succ_token);
            free(temp_succ_token_key);
            temp_succ_token_key = NULL;

            //printf("\nNow searching for %s in hash\n", temp_hash_key->key_token);

            temp_hash = bsearch(temp_hash_key, map_pkg->token_hash, map_pkg->hash_size,
                                sizeof(token_hash_map), comp_token_hash);

            if (temp_hash == NULL) {
                printf("Not able to find next token in hash! Possible programming error in probability distribution!\n");
                err_flag = 1;
                break;
            } else {
                temp_list = temp_hash->token_location;
                // Print word
                if (choice == 1) {
                    printf(" %s", temp_list->token);
                } else {
                    fprintf(out_handle, " %s", temp_list->token);
                }

                free(temp_hash_key);
                temp_hash_key = NULL;
            }
        } else {
            printf("Not able to find next token! Possible programming error in probability distribution!\n");
            err_flag = 1;
            break;
        }
    }

    if (choice == 1) {
        printf("\n");
    } else {
        printf("\nFinished writing to %s.\n", file_name);
        fclose(out_handle);
    }
    return err_flag;
}

// Function     : clean_up_token_list()
// Description  : This function cleans up the token list.
// Parameters   : token_list_ptr - pointer to token list processed from input text
// Return Value : None
void clean_up_token_list(curr_token_ptr *token_list) {
    curr_token_ptr temp_list = NULL;

    while (*token_list != NULL) {
        temp_list = *token_list;
        // Clean up succeeding token list
        if ((*token_list)->succ_token_list != NULL) {
            free((*token_list)->succ_token_list);
            (*token_list)->succ_token_list = NULL;
        }
        // Traverse to the next list
        *token_list = (*token_list)->next;
        // Free up the token list entry
        free(temp_list);
        temp_list = NULL;
    }
}
