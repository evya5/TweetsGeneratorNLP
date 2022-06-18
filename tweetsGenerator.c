#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORDS_IN_SENTENCE_GENERATION 20
#define MAX_WORD_LENGTH 100
#define MAX_SENTENCE_LENGTH 1000

#define ALLOC_ERROR_MSG "Allocation failure: System's fault\n"
#define SPACE " "
#define FILE_ERROR_MSG "Error: File Not found\n"
#define TITLE_OF_TWEET "Tweet %d: "
#define USAGE_MSG "Usage: <SEED> <AMOUNT OF TWEETS TO GENERATE> "\
                     "<PATH TO TEXT FILE> "\
                     "Optional: <AMOUNT OF WORDS TO READ>\n"
#define BASE 10
#define FIVE_AGRS 5
#define FOUR_ARGS 4
#define THREE 3
int error_indicator = 0;

typedef struct WordStruct {
    char *word;
    struct WordProbability *prob_list;
    int is_last;
    int all_words_counter;
    int diff_words_counter;
    //... Add your own fields here
} WordStruct;

typedef struct WordProbability {
    struct WordStruct *word_struct_ptr;
    int appearances;
    int index;
    //... Add your own fields here
} WordProbability;

/************ LINKED LIST ************/
typedef struct Node {
    WordStruct *data;
    struct Node *next;
} Node;

typedef struct LinkList {
    Node *first;
    Node *last;
    int size;
} LinkList;

/**
 * Add data to new node at the end of the given link list.
 * @param link_list Link list to add data to
 * @param data pointer to dynamically allocated data
 * @return 0 on success, 1 otherwise
 */
int add (LinkList *link_list, WordStruct *data)
{
    Node *new_node = malloc (sizeof (Node));
    if (new_node == NULL)
        {
            return 1;
        }
    *new_node = (Node) {data, NULL};

    if (link_list->first == NULL)
        {
            link_list->first = new_node;
            link_list->last = new_node;
        }
    else
        {
            link_list->last->next = new_node;
            link_list->last = new_node;
        }

    link_list->size++;
    return 0;
}
/*************************************/

void print_prob_list (WordStruct *word_struct)
{
    if (word_struct->prob_list == NULL)
        {
            return;
        }
    int i = 0;
    while (i < word_struct->diff_words_counter)
        {
            fprintf (stdout, "\t%s - %d\n",
                     word_struct->prob_list[i].word_struct_ptr->word,
                     word_struct->prob_list[i].appearances);
            i++;
        }
}

void print_dictionary (LinkList *dictionary)
{
    Node *runner = dictionary->first;
    while (runner != NULL)
        {
            fprintf (stdout, "%s :  ", runner->data->word);
            fprintf (stdout, "%d\n", runner->data->all_words_counter);
            print_prob_list (runner->data);
            fprintf (stdout, "---\n");
            runner = runner->next;
        }
    fprintf (stdout, "Words in total: %d\n", dictionary->size);
}



/**
 * This function initialize the word_struct with the given values.
 * @param word_struct - word_struct to initialize.
 * @param word - the word it will contain
 * @param is_last - determines if the word ends with DOT or isn't.
 */
void apply_word_details (WordStruct *word_struct, char *word, int is_last)
{
    word_struct->word = word;
    word_struct->is_last = is_last;
    word_struct->prob_list = NULL;
    word_struct->all_words_counter = 0;
    word_struct->diff_words_counter = 0;
}

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number (int max_number)
{
    int rand_num = rand () % max_number;
    return rand_num;
}

/**
 * Choose randomly the next word from the given dictionary, drawn uniformly.
 * The function won't return a word that end's in full stop '.' (Nekuda).
 * @param dictionary Dictionary to choose a word from
 * @return WordStruct of the chosen word
 */
WordStruct *get_first_random_word (LinkList *dictionary)
{
    int random_num = get_random_number (dictionary->size);
    int cur_index = 0;
    Node *runner = dictionary->first;
    while (random_num >= cur_index)
        {
            if (!(runner->data->is_last))
                {
                    if (random_num == cur_index)
                        {
                            return runner->data;
                        }
                    else
                        {
                            cur_index++;
                            runner = runner->next;
                            if (runner == NULL)
                                {
                                    runner = dictionary->first;
                                }
                        }
                }
            else
                {
                    runner = runner->next;
                    if (runner == NULL)
                        {
                            runner = dictionary->first;
                        }
                }
        }
    return NULL;
}

/**
 * This function builds the array which contains every word in the probability
 * list the amount of times it appears after the Word_struct word.
 * @param word_struct_ptr
 * @param all_options - an array the size of the number of  appearances of the
 * word.
 */
void build_options (WordStruct *word_struct_ptr, WordProbability *all_options)
{
    WordProbability *runner = all_options;
    for (int i = 0; i < word_struct_ptr->diff_words_counter; i++)
        {
            for (int j = 0; j < word_struct_ptr->prob_list[i].appearances; j++)
                {
                    runner->word_struct_ptr = word_struct_ptr->prob_list[
                        i].word_struct_ptr;
                    runner++;
                }
        }
}

/**
 * Choose randomly the next word. Depend on it's occurrence frequency
 * in word_struct_ptr->WordProbability.
 * @param word_struct_ptr WordStruct to choose from
 * @return WordStruct of the chosen word
 */
WordStruct *get_next_random_word (WordStruct *word_struct_ptr)
{
    int random_num = get_random_number (word_struct_ptr->all_words_counter);
    struct WordProbability *all_options = malloc (
        word_struct_ptr->all_words_counter * sizeof (WordProbability));
    if (all_options == NULL)
        {
            fprintf (stdout, ALLOC_ERROR_MSG);
            error_indicator = -1;
            return NULL;
        }

    build_options (word_struct_ptr, all_options);
    WordStruct *chosen_word = all_options[random_num].word_struct_ptr;
    free (all_options);
    return chosen_word;

}

/**
 * Receive dictionary, generate and print to stdout random sentence out of it.
 * The sentence most have at least 2 words in it.
 * @param dictionary Dictionary to use
 * @return Amount of words in printed sentence
 */
int generate_sentence (LinkList *dictionary)
{
    int words_amount = 1;
    WordStruct *word = get_first_random_word (dictionary);
    if (word == NULL)
        return 0;
    fprintf (stdout, "%s", word->word);
    while (!(word->is_last)
           && (words_amount < MAX_WORDS_IN_SENTENCE_GENERATION))
        {
            words_amount++;
            word = get_next_random_word (word);
            if (error_indicator)
                {
                    return 0;
                }
            fprintf (stdout, " %s", word->word);
        }
    return words_amount;
}

/**
 * This function checks if a word is already in the probability list or isn't
 * @param first_word - the word which conatins the prob_list needed to be
 * examined.
 * @param second_word - the word which needs to be evaluate.
 * @return NULL if the word isn't in the probability list and the ptr of the
 * if it is already in the list.
 */
WordProbability *not_in_prob_list (WordStruct *first_word,
                                   WordStruct *second_word)
{
    int i = 0;
    while (i < first_word->diff_words_counter)
        {
            if (!(strcmp (first_word->prob_list[i].word_struct_ptr->word,
                          second_word->word)))
                {
                    return &first_word->prob_list[i];
                }
            i++;
        }
    return NULL;
}

/**
 * Gets 2 WordStructs. If second_word in first_word's prob_list,
 * update the existing probability value.
 * Otherwise, add the second word to the prob_list of the first word.
 * @param first_word
 * @param second_word
 * @return 0 if already in list, 1 otherwise.
 */
int add_word_to_probability_list (WordStruct *first_word,
                                  WordStruct *second_word)
{
    WordProbability *word_in_prob_list;
    if (!first_word->is_last)
        {
            if (first_word->prob_list == NULL
                && strcmp (first_word->word, " ") != 0)
                {
                    first_word->prob_list = (WordProbability *)
                        malloc (sizeof (WordProbability));
                    if (first_word->prob_list == NULL)
                        {
                            fprintf (stdout, ALLOC_ERROR_MSG);
                            error_indicator = -1;
                            return 1;
                        }
                    first_word->all_words_counter++;
                    first_word->diff_words_counter++;
                    first_word->prob_list->word_struct_ptr = second_word;
                    first_word->prob_list->appearances = 1;
                    first_word->prob_list->index = 0;
                    return 1;
                }
            else if (!(word_in_prob_list = not_in_prob_list (first_word,
                                                             second_word)))
                {
                    first_word->all_words_counter++;
                    first_word->diff_words_counter++;
                    first_word->prob_list = (WordProbability *)
                        realloc (first_word->prob_list,
                                 (first_word->diff_words_counter)
                                 * sizeof (WordProbability));
                    if (first_word->prob_list == NULL)
                        {
                            fprintf (stdout, ALLOC_ERROR_MSG);
                            error_indicator = -1;
                            return 1;
                        }
                    first_word->prob_list[first_word->diff_words_counter
                                          - 1].word_struct_ptr = second_word;
                    first_word->prob_list[first_word->diff_words_counter
                                          - 1].appearances = 1;
                    first_word->prob_list[first_word->diff_words_counter
                                          - 1].index =
                        first_word->diff_words_counter - 1;
                    return 1;
                }
            else
                {
                    first_word->all_words_counter++;
                    first_word->prob_list[
                        word_in_prob_list->index].appearances++;
                    return 0;
                }

        }
    return 1;

}

/**
 * Checks if the word is already in the dictionary.
 * @param word - the new word read from the file.
 * @param dictionary - the dictionary which contains all the words until this
 * point.
 * @return NULL if the word isn't in the dictionary and the ptr of the word
 * if it is already in it.
 */
Node *not_in_dict (char *word, LinkList *dictionary)
{
    Node *runner = dictionary->first;
    while (runner != NULL)
        {
            if (!strcmp (runner->data->word, word))
                {
                    return runner;
                }
            runner = runner->next;
        }
    return NULL;
}

/**
 * this function updates the dictionary with the new word the was read
 * depending on the case.
 * @param word - the new word
 * @param len_word - the length of it.
 * @param dictionary - the dictionary that is being built.
 * @param prev_ptr - the last word_struct modified (in order to update it's
 * probability list).
 * @return a pointer to the new Node or a pointer to the updated node which
 * was already in the dictionary.
 */
Node *
update_dict (char *word, int len_word, LinkList *dictionary, Node *prev_ptr)
{
    int is_last = 0;
    if (word[len_word - 1] == '.' || word[len_word - 1] == '\n')
        {
            sscanf (word, "%[^\n]", word);
            is_last = 1;
        }
    Node *word_ptr = not_in_dict (word, dictionary);
    if (word_ptr == NULL)
        {
            char *word_in_memory = (char *) malloc (strlen (word) + 1);
            if (word_in_memory == NULL)
                {
                    fprintf (stdout, ALLOC_ERROR_MSG);
                    error_indicator = -1;
                    return NULL;
                }
            strcpy (word_in_memory, word);
            WordStruct *word_struct = (WordStruct *)
                malloc (sizeof (WordStruct));
            if (word_struct == NULL)
                {
                    fprintf (stdout, ALLOC_ERROR_MSG);
                    error_indicator = -1;
                    return NULL;
                }

            apply_word_details (word_struct, word_in_memory, is_last);
            if (add (dictionary, word_struct) == 1)
                {
                    fprintf (stdout, ALLOC_ERROR_MSG);
                    error_indicator = -1;
                    return NULL;
                }
            add_word_to_probability_list (prev_ptr->data,
                                          dictionary->last->data);
            if (error_indicator)
                {
                    return NULL;
                }
            return (dictionary->last);
        }
    else
        {
            add_word_to_probability_list (prev_ptr->data, word_ptr->data);
            if (error_indicator)
                {
                    return NULL;
                }
            return word_ptr;
        }
}

/**
 * Read word from the given file. Add every unique word to the dictionary.
 * Also, at every iteration, update the prob_list of the previous word with
 * the value of the current word.
 * @param fp File pointer
 * @param words_to_read Number of words to read from file.
 *                      If value is bigger than the file's word count,
 *                      or if words_to_read == -1 than read entire file.
 * @param dictionary Empty dictionary to fill
 */
void fill_dictionary (FILE *fp, int words_to_read, LinkList *dictionary)
{
    char line[MAX_SENTENCE_LENGTH];
    char *word_in_line;
    int len_word;
    char *empty_word_struct = " ";
    WordStruct null_struct;
    Node *prev_ptr = (Node *) malloc (sizeof (Node));
    Node *temp = prev_ptr;
    if (prev_ptr == NULL)
        {
            fprintf (stdout, ALLOC_ERROR_MSG);
            error_indicator = -1;
            return;
        }
    apply_word_details (&null_struct, empty_word_struct, -1);
    prev_ptr->data = &null_struct;
    while (!feof(fp))
        {
            if (fgets (line, MAX_SENTENCE_LENGTH, fp) != NULL)
                {
                    word_in_line = strtok (line, SPACE);
                    len_word = strlen (word_in_line);
                    while (word_in_line != NULL && words_to_read)
                        {
                            if (words_to_read > 0)
                                {
                                    words_to_read--;
                                }
                            prev_ptr = update_dict (word_in_line, len_word,
                                                    dictionary, prev_ptr);
                            if (error_indicator)
                                {
                                    return;
                                }
                            word_in_line = strtok (NULL, SPACE);
                            if (word_in_line != NULL)
                                {
                                    len_word = strlen (word_in_line);
                                }
                        }
                    if (words_to_read == 0)
                        {
                            break;
                        }
                }
        }
    free (temp);
}

/**
 * Free the given dictionary and all of it's content from memory.
 * @param dictionary Dictionary to free
 */
void free_dictionary (LinkList *dictionary)
{
    Node *runner = dictionary->first;
    while (runner != NULL)
        {
            if (runner->data->prob_list != NULL)
                {
                    free (runner->data->prob_list);
                }
            if (runner->data->word != NULL)
                {
                    free (runner->data->word);
                }
            free (runner->data);
            Node *temp = runner;
            runner = runner->next;
            free (temp);
        }
    free (dictionary);
}

/**
 * This function initializes the linked list with the first values.
 * @param dictionary
 */
void init_dictionary (LinkList *dictionary)
{
    dictionary->size = 0;
    dictionary->first = NULL;
    dictionary->last = NULL;
}

/**
 * @param argc
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 *             3) Path to file
 *             4) Optional - Number of words to read
 */
int main (int argc, char *argv[])
{
    if (argc == FOUR_ARGS || argc == FIVE_AGRS)
        {
            int words_to_read = -1;
            int amount_of_sentences = (int) strtol (argv[2], NULL, BASE);
            if (argc == FIVE_AGRS)
                {
                    words_to_read = (int) strtol (argv[FIVE_AGRS - 1], NULL,
                                                  BASE);
                }
            FILE *text_file = fopen (argv[FOUR_ARGS - 1], "r");
            if (text_file == NULL)
                {
                    fprintf (stdout, FILE_ERROR_MSG);
                    return EXIT_FAILURE;
                }
            LinkList *dictionary = (LinkList *) malloc (sizeof (LinkList));
            if (dictionary == NULL)
                {
                    fprintf (stdout, ALLOC_ERROR_MSG);
                    return EXIT_FAILURE;
                }
            init_dictionary (dictionary);
            fill_dictionary (text_file, words_to_read, dictionary);
//            print_dictionary(dictionary);
            fclose (text_file);
            if (error_indicator)
                {
                    return EXIT_FAILURE;
                }
            srand (strtol (argv[1], NULL, BASE));
            for (int i = 1; i <= amount_of_sentences; i++)
                {
                    fprintf (stdout, TITLE_OF_TWEET, i);
                    generate_sentence (dictionary);
                    if (error_indicator)
                        {
                            EXIT_FAILURE;
                        }
                    fprintf (stdout, "\n");
                }
            free_dictionary (dictionary);
            text_file = NULL;
            return EXIT_SUCCESS;
        }
    else
        {
            fprintf (stdout, USAGE_MSG);
            return EXIT_FAILURE;
        }
}