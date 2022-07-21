#include <stdio.h>
#include <stdlib.h>

enum Actions
{
    NEW_GAME,
    QUIT,
    READ_WORDS,
};

struct char_occurrence
{
    char character;
    int position;
};

struct node
{
    struct node *next;
    char word[350];
};

struct c_o_node
{
    struct c_o_node *next;
    struct char_occurrence occurrence;
};

typedef struct c_o_node *occurrrences_list;

typedef struct user_knowledge
{
    /// For know why 64 see indexOfCharacter(...) method
    /// For example: if excluded_characters[0] == 0 then "_" can be in the secret string
    /// Or: if excluded_characters[5] == 1 then "4" cannot be in the secret string
    int excluded_characters[64];
    /// Those two lists rapresent the occurrences (ie: char and position)
    /// where the user know if is rigth or wrong
    occurrrences_list rigth_spots;
    occurrrences_list wrong_spots;
    /// Same as for excluded_characters
    /// For example: if min_times[0] == -1 then user doesn't know the min times "_" occurres in the result string
    /// Or: if exact_times[5] == 3 then user know that "4" appears exaclty 4 times in the string
    int min_times[64];
    int exact_times[64];
} user_knowledge;

typedef struct node *list;

int strLength(char *str);
void stringCopy(char *original, char *dest);
int fillDictionary(list *dictionary, int length);
void stampaLista(list l);
list insertNewWordToList(list l, char *word);
int stringEqual(char *s1, char *s2);
int listContain(list l, char *word);
int stringContains(char *string, char character);
void printHint(char *secret, char *inserted, int word_length, user_knowledge *knowledge);
int indexOfCharacter(char character);
void newGame(list *dict, int word_length);
enum Actions checkNextAction();
user_knowledge *getEmptyKnowledge();
int occurrenceListContains(occurrrences_list list, struct char_occurrence occurrence);
void addRigthOccurrenceToKnowledge(user_knowledge *knowledge, char character, int position);
void addWrongOccurrenceToKnowledge(user_knowledge *knowledge, char character, int position);
int countOccurrenceInString(char *string, char character);
int min(int a, int b);
int max(int a, int b);
list copyList(list a, int word_length);
int evaluateKnowledgeOnWord(char *word, user_knowledge knowledge);
void removeWordFromList(list *l, char *word);
int getLengthOfList(list l);
list filterDictionary(list dict, user_knowledge knowledge);
char characterOfIndex(int index);
int compareStrings(char *s1, char *s2);
void insertNewWords(list *main_dictionary, list *filtered_dic, user_knowledge *knowledge, int word_lenght);
void clearList(list l);
void clearKnowledge(user_knowledge *knowledge);
void clearOccList(occurrrences_list list);
list firstFilterDictionary(list *dict, user_knowledge knowledge);
void stampaKnowledge(user_knowledge k);

int main()
{
    list l = NULL;

    /// Used to detect a new game string
    int continueGame = 0;

    int word_length = 0;
    int l1 = scanf("%d", &word_length);
    l1 = l1 + 1;

    int start_game = fillDictionary(&l, word_length);

    if (start_game)
        newGame(&l, word_length);

    continueGame = checkNextAction();

    while (continueGame != QUIT)
    {
        if (continueGame == NEW_GAME)
        {
            newGame(&l, word_length);
        }
        else if (continueGame == READ_WORDS)
        {
            insertNewWords(&l, NULL, NULL, word_length);
        }

        continueGame = checkNextAction();
    }

    return 0;
}

enum Actions checkNextAction()
{
    char input[1000];
    int size = 0;
    size = scanf("%s", input);

    if (size == EOF)
        return QUIT;

    if (stringEqual(input, "+nuova_partita") == 1)
        return NEW_GAME;
    if (stringEqual(input, "+inserisci_inizio") == 1)
        return READ_WORDS;

    return QUIT;
}

void newGame(list *dict, int word_length)
{
    user_knowledge *knownledge = getEmptyKnowledge();
    list filtered_dict = NULL;
    int attempts = 0;
    char secret[word_length + 1];

    int l1 = scanf("%s", secret);
    int l2 = scanf("%d", &attempts);

    l1 = l1 + l2;

    int i = 0;

    /// Has 3 different values: 0: stop the game, 1: continue the game
    /// 2: continue tha game but only for wipe the moves after a user fail (ko).
    int continue_game = 1;
    while (continue_game != 0)
    {
        i++;
        char inserted[1000];
        int l3 = scanf("%s", inserted);

        l3 += l3 % 2 == 0 ? 1 : -1;

        if (stringEqual(inserted, "+stampa_filtrate"))
        {
            if (filtered_dict == NULL)
            {
                stampaLista(*dict);
            }
            else
            {
                stampaLista(filtered_dict);
            }
        }
        else if (stringEqual(inserted, "+inserisci_inizio"))
        {
            if (filtered_dict == NULL)
            {
                insertNewWords(dict, NULL, knownledge, word_length);
            }
            else
            {
                insertNewWords(dict, &filtered_dict, knownledge, word_length);
            }
        }

        else if (stringEqual(secret, inserted) == 1)
        {
            /// User win
            continue_game = 0;
            printf("ok\n");
        }
        else if (listContain(*dict, inserted) == 0)
        {
            printf("not_exists\n");
        }
        else
        {
            attempts--;

            printHint(secret, inserted, word_length, knownledge);
            if (filtered_dict == NULL)
            {
                filtered_dict = firstFilterDictionary(dict, *knownledge);
            }
            else
            {
                filtered_dict = firstFilterDictionary(&filtered_dict, *knownledge);
            }

            // stampaKnowledge(*knownledge);

            printf("%d\n", getLengthOfList(filtered_dict));
        }

        if (attempts == 0)
        {
            printf("ko\n");
            continue_game = 0;
            clearList(filtered_dict);
            clearKnowledge(knownledge);
            return;
        }
    }

    clearList(filtered_dict);
    clearKnowledge(knownledge);
    return;
}

/// Different possible characters: 64 = 26 (lower case) + 26 (upper case) + 10 (numbers) + 2 (dash, underscore)

/// Prints the hint in base of the secret and
/// the inserted
void printHint(char *secret, char *inserted, int word_length, user_knowledge *knowledge)
{
    /// For example: available_characters[0] is the times "-" appeared in the secret string
    /// and available_characters[16] is the times "F" appeared in the secred string
    ///
    /// see indexOfCharacter(...) for details
    int available_characters[64] = {0};
    int all_character_occurrence[64] = {0};
    int rigth_predictions_characters[64] = {0};
    char result[word_length + 1];
    int i = 0;

    for (; i < word_length; i++)
    {
        int index_of_character_s = indexOfCharacter(secret[i]);

        all_character_occurrence[index_of_character_s]++;
        if (secret[i] == inserted[i])
        {
            addRigthOccurrenceToKnowledge(knowledge, secret[i], i);
            rigth_predictions_characters[index_of_character_s]++;
            result[i] = '+';
        }
        else
        {
            available_characters[index_of_character_s]++;
            result[i] = '?';
        }
    }

    int pipeline_added[64] = {0};

    for (i = 0; i < word_length; i++)
    {
        if (result[i] != '?')
            continue;

        int index_of_character_i = indexOfCharacter(inserted[i]);

        if (stringContains(secret, inserted[i]) == 0)
        {
            knowledge->excluded_characters[index_of_character_i] = 1;
            addWrongOccurrenceToKnowledge(knowledge, inserted[i], i);
            result[i] = '/';
            continue;
        }

        if (available_characters[index_of_character_i] == 0)
        {
            knowledge->exact_times[index_of_character_i] = rigth_predictions_characters[index_of_character_i] + pipeline_added[index_of_character_i];
            addWrongOccurrenceToKnowledge(knowledge, inserted[i], i);
            result[i] = '/';
        }
        else
        {
            result[i] = '|';
            pipeline_added[index_of_character_i]++;

            addWrongOccurrenceToKnowledge(knowledge, inserted[i], i);
            int current_min_times = knowledge->min_times[index_of_character_i];

            knowledge->min_times[index_of_character_i] = max(current_min_times, rigth_predictions_characters[index_of_character_i] + pipeline_added[index_of_character_i]);
            available_characters[index_of_character_i]--;
        }
    }

    printf("%s\n", result);
}

/// To waste less time, instead of copy the original list
/// into a filterer_list the first time (ie: filtered_list is NULL)
/// we use the original list to first filter and save
/// items inside filtered list. Which will be used the next times
list firstFilterDictionary(list *dict, user_knowledge knowledge)
{
    struct node *n = *dict;

    list res = NULL;

    while (n != NULL)
    {
        if (evaluateKnowledgeOnWord(n->word, knowledge) == 1)
        {
            res = insertNewWordToList(res, n->word);
            n = n->next;
        }
        else
            n = n->next;
    }

    return res;
}

list filterDictionary(list dict, user_knowledge knowledge)
{
    if (dict == NULL)
        return NULL;

    list head = dict;

    struct node *in = NULL;
    struct node *n = dict;

    while (n != NULL)
    {
        if (evaluateKnowledgeOnWord(n->word, knowledge) == 0)
        {
            if (in != NULL)
            {
                in->next = n->next;
            }
            else
            {
                if (n->next == NULL)
                {
                    return NULL;
                }
                head = n->next;
            }

            n = n->next;
        }
        else
        {
            in = n;
            n = n->next;
        }
    }

    return head;
}

/// Called when stdin reads '+inserici_inizio
/// If filtered_dic and knowledge are not null, the new workd
/// are filtered "in-real-time" and inserted in both lists
void insertNewWords(list *main_dictionary, list *filtered_dic, user_knowledge *knowledge, int word_lenght)
{
    char inserted[50];
    int size;

    while ((size = scanf("%s", inserted)) != EOF)
    {
        if (stringEqual(inserted, "+inserisci_fine"))
            break;

        /// Otherwise we are adding a word so:
        *main_dictionary = insertNewWordToList(*main_dictionary, inserted);
        if (filtered_dic != NULL && knowledge != NULL && evaluateKnowledgeOnWord(inserted, *knowledge) == 1)
            *filtered_dic = insertNewWordToList(*filtered_dic, inserted);
    }
}

/// Check all the user_knowledge conditions on the provided string
/// Return 1 if string is ok, 0 otherwise
int evaluateKnowledgeOnWord(char *word, user_knowledge knowledge)
{
    for (int i = 0; i < 64; i++)
    {
        /// String contains the i-th character
        if (knowledge.excluded_characters[i] == 1 && stringContains(word, characterOfIndex(i)) == 1)
        {
            return 0;
        }

        /// String has different count of i-th character
        if (knowledge.exact_times[i] != -1 && countOccurrenceInString(word, characterOfIndex(i)) != knowledge.exact_times[i])
        {
            return 0;
        }

        /// String has less i-th character than min
        if (knowledge.min_times[i] != -1 && countOccurrenceInString(word, characterOfIndex(i)) < knowledge.min_times[i])
        {
            return 0;
        }
    }

    struct c_o_node *r_n = knowledge.rigth_spots;
    while (r_n != NULL)
    {
        if (word[r_n->occurrence.position] != r_n->occurrence.character)
        {
            return 0;
        }
        r_n = r_n->next;
    }

    struct c_o_node *w_n = knowledge.wrong_spots;
    while (w_n != NULL)
    {
        if (word[w_n->occurrence.position] == w_n->occurrence.character)
        {
            return 0;
        }
        w_n = w_n->next;
    }

    return 1;
}

void stampaLista(list l)
{
    struct node *x = l;
    while (x != NULL)
    {
        printf("%s\n", x->word);
        x = x->next;
    }
}

/// Return 1 if a new game should start, 0 otherwise
int fillDictionary(list *dictionary, int length)
{
    int continue_reading = 1;
    while (continue_reading == 1)
    {
        char string[length + 1];
        int size = 0;

        size = scanf("%s", string);
        if (size == EOF || size == 0)
        {
            continue_reading = 0;
        }
        else if (string[0] == '+')
        {
            char c[] = "+nuova_partita";
            if (stringEqual(string, c) == 1)
                return 1;
        }
        else if (strLength(string) != length)
        {
            printf("Errore nella lettura dell'input: La stringa fornita non rispetta il vincolo riguardo la lunghezza");
        }
        else
        {
            string[length] = '\0';
            *dictionary = insertNewWordToList(*dictionary, string);
        }
    }

    return 0;
}

/// Insert the given word in a lexilogical order
list insertNewWordToList(list l, char *word)
{
    struct node *n = malloc(sizeof(struct node));
    n->next = NULL;
    stringCopy(word, n->word);

    if (l == NULL)
        return n;

    struct node *inseguitore = NULL;
    struct node *x = l;

    while (x != NULL && (compareStrings(word, x->word) > 0))
    {
        inseguitore = x;
        x = x->next;
    }

    if (inseguitore != NULL)
    {
        inseguitore->next = n;
        n->next = x;
        return l;
    }

    n->next = l;
    return n;
}

/// Compare two strings, if s1 ==  s2 retun 0;
/// if s1 > s2 return 1, -1 otherwise
int compareStrings(char *s1, char *s2)
{
    while (*s1)
    {
        if (*s1 != *s2)
            break;
        s1++;
        s2++;
    }

    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strLength(char *str)
{
    char *s;
    for (s = str; *s; ++s)
        ;
    return (s - str);
}

void stringCopy(char *original, char *dest)
{
    if (dest == NULL)
        return;
    int i = 0;
    for (; original[i] != '\0'; i++)
        dest[i] = original[i];
    dest[i + 1] = '\0';
}

/// Return 1 if s1 is equal to s2, 0 otherwise
int stringEqual(char *s1, char *s2)
{
    int i = 0;

    for (; s1[i] != '\0'; i++)
    {
        if (s1[i] != s2[i])
            return 0;
    }

    if (s2[i] == '\0')
        return 1;
    return 0;
}

/// Return 1 if the provided string contains at least one character
int stringContains(char *string, char character)
{
    for (int i = 0; string[i] != '\0'; i++)
        if (string[i] == character)
            return 1;
    return 0;
}

/// Check if the provided list l contains the second
/// arument: word
///
/// If there is a match (ie: list contains word)
/// return 1, return 0 otherwise
int listContain(list l, char *word)
{
    if (stringEqual(l->word, word) == 1)
        return 1;

    struct node *n = l->next;
    while (n != NULL)
    {
        if (stringEqual(n->word, word) == 1)
            return 1;
        n = n->next;
    }

    return 0;
}

/// In base of ASCII order: "-" < 0-9 < A-Z < "_" < a-z
///
/// So '-' is at index 0, "0" at index 1, "A" at index 11 and "z" at index 63
int indexOfCharacter(char character)
{
    int ascii = (int)character;

    if (ascii == 45)
        return 0;
    if (ascii >= 48 && ascii <= 57)
        return ascii - 48 + 1;
    if (ascii >= 65 && ascii <= 90)
        return ascii - 65 + 11;
    if (ascii == 95)
        return 37;
    if (ascii >= 97 && ascii <= 122)
        return ascii - 97 + 38;

    return -1;
}

/// Opposite of indexOfCharacter(...) method
char characterOfIndex(int index)
{
    if (index == 0)
        return '-';
    if (index >= 1 && index <= 10)
        return (char)(index + 47);
    if (index >= 11 && index <= 36)
        return (char)(index + 54);
    if (index == 37)
        return '_';
    if (index >= 38 && index <= 63)
        return (char)(index + 59);

    return '\0';
}

user_knowledge *getEmptyKnowledge()
{
    user_knowledge *knownledge = malloc(sizeof(user_knowledge));

    for (int i = 0; i < 64; i++)
    {
        knownledge->excluded_characters[i] = 0;
        knownledge->min_times[i] = -1;
        knownledge->exact_times[i] = -1;
    }

    knownledge->rigth_spots = NULL;
    knownledge->wrong_spots = NULL;

    return knownledge;
}

void addRigthOccurrenceToKnowledge(user_knowledge *knowledge, char character, int position)
{
    struct char_occurrence *occurrence = malloc(sizeof(occurrence));
    occurrence->character = character;
    occurrence->position = position;

    if (knowledge->rigth_spots == NULL)
    {
        struct c_o_node *new_node = malloc(sizeof(struct c_o_node));
        new_node->occurrence = *occurrence;

        knowledge->rigth_spots = new_node;
        return;
    }

    if (occurrenceListContains(knowledge->rigth_spots, *occurrence) == 1)
        return;
    struct c_o_node *n = knowledge->rigth_spots;
    while (n->next != NULL)
        n = n->next;

    struct c_o_node *new_node = malloc(sizeof(struct c_o_node));
    new_node->occurrence = *occurrence;
    n->next = new_node;
}

void addWrongOccurrenceToKnowledge(user_knowledge *knowledge, char character, int position)
{
    struct char_occurrence *occurrence = malloc(sizeof(occurrence));
    occurrence->character = character;
    occurrence->position = position;

    if (knowledge->wrong_spots == NULL)
    {
        struct c_o_node *new_node = malloc(sizeof(struct c_o_node));
        new_node->occurrence = *occurrence;

        knowledge->wrong_spots = new_node;
        return;
    }

    if (occurrenceListContains(knowledge->wrong_spots, *occurrence) == 1)
        return;
    struct c_o_node *n = knowledge->wrong_spots;
    while (n->next != NULL)
        n = n->next;

    struct c_o_node *new_node = malloc(sizeof(struct c_o_node));
    new_node->occurrence = *occurrence;
    n->next = new_node;
}

/// Return 1 is the provided list contains the provided occurrence
int occurrenceListContains(occurrrences_list list, struct char_occurrence occurrence)
{
    if (list == NULL)
        return 0;
    struct c_o_node *n = list;
    while (n != NULL)
    {
        if (n->occurrence.character == occurrence.character && n->occurrence.position == occurrence.position)
            return 1;

        n = n->next;
    }
    return 0;
}

/// Return the count of the times the character occurres in string
int countOccurrenceInString(char *string, char character)
{
    int counter = 0;
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] == character)
            counter++;
    }
    return counter;
}

/// Return the smallest between a and b
int min(int a, int b)
{
    if (a > b)
        return b;
    return a;
}

/// Return the biggest between a and b
int max(int a, int b)
{
    if (a < b)
        return b;
    return a;
}

list copyList(list a, int word_length)
{
    if (a == NULL)
        return NULL;

    struct node *new_list = NULL;

    struct node *n = a;
    while (n != NULL)
    {
        new_list = insertNewWordToList(new_list, n->word);
        n = n->next;
    }

    return new_list;
}

void removeWordFromList(list *l, char *word)
{
    if (l == NULL)
        return;

    struct node *in = NULL;
    struct node *n = *l;

    while (n != NULL)
    {
        if (stringEqual(n->word, word) == 1)
        {
            if (in != NULL)
                in->next = n->next;
            free(n);
            return;
        }

        in = n;
        n = n->next;
    }
}

int getLengthOfList(list l)
{
    int counter = 0;

    struct node *n = l;
    while (n != NULL)
    {
        counter++;
        n = n->next;
    }

    return counter;
}

void clearList(list l)
{
    struct node *n = l;
    while (n != NULL)
    {
        struct node *tmp = n;
        n = n->next;
        free(tmp);
    }
}

void clearKnowledge(user_knowledge *knowledge)
{
    clearOccList(knowledge->rigth_spots);
    clearOccList(knowledge->wrong_spots);
    free(knowledge);
}

void clearOccList(occurrrences_list list)
{
    struct c_o_node *n = list;
    while (n != NULL)
    {
        struct c_o_node *tmp = n;
        n = n->next;
        free(tmp);
    }
}

void stampaKnowledge(user_knowledge k)
{
    printf("\nKnowledge:\n");
    for (int i = 0; i < 64; i++)
    {
        char c = characterOfIndex(i); 
        printf("Char %c: %d - %d - %d\n", c, k.exact_times[i], k.min_times[i], k.excluded_characters[i]);
    }

    printf("Rigth positions:\n");
    struct c_o_node *n1 = k.rigth_spots;
    while (n1 != NULL)
    {
        printf("%c: %d\n", n1->occurrence.character, n1->occurrence.position);
        n1 = n1->next;
    }

    printf("Wrong positions:\n");
    struct c_o_node *n2 = k.wrong_spots;
    while (n2 != NULL)
    {
        printf("%c: %d\n", n2->occurrence.character, n2->occurrence.position);
        n2 = n2->next;
    }
}