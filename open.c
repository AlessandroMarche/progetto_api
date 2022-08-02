#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 89
#define S_LEN 10

int word_length;

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

typedef struct table_item
{
    char str[S_LEN];
    unsigned char valid; // 0: invalid, 1: valid
} table_item;

struct list_item
{
    struct list_item *next;
    char word[S_LEN];
};

typedef struct list_item *list;

table_item *table[SIZE];

int indexOfCharacter(char character);
unsigned long hash(char str[S_LEN]);
void sortAndPrintList(char items[][S_LEN], int count);
void quickSortMain(char items[][S_LEN], int count);
void quickSort(char items[][S_LEN], int left, int right);
int countValidItems();
void filterTable(user_knowledge *knowledge);
int evaluateKnowledgeOnWord(char *word, user_knowledge knowledge);
int checkIfExixsts(char *string);
void setAllStringAllValid();
int fillDictionary(int length);
void newGame(int word_length);
enum Actions checkNextAction();
user_knowledge *getEmptyKnowledge();
void addRigthOccurrenceToKnowledge(user_knowledge *knowledge, char character, int position);
void addWrongOccurrenceToKnowledge(user_knowledge *knowledge, char character, int position);
void printHint(char *secret, char *inserted, int word_length, user_knowledge *knowledge);
void clearKnowledge(user_knowledge *knowledge);
int stringContains(char *string, char character);
int min(int a, int b);
int max(int a, int b);
int occurrenceListContains(occurrrences_list list, struct char_occurrence occurrence);
void clearOccList(occurrrences_list list);
int countOccurrenceInString(char *string, char character);
void stampaTabella();

unsigned long hash(char str[S_LEN])
{
    unsigned long hash = 5381;
    int c;
    for (int i = 0; i < S_LEN && str[i] != '\n'; i++)
    {
        c = str[i];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % SIZE;
}

int main()
{
    /// Used to detect a new game string
    int continueGame = 0;

    int l1 = scanf("%d", &word_length);
    l1 = l1 + 1;

    int start_game = fillDictionary(word_length);
    exit(0);

    if (start_game)
        newGame(word_length);

    continueGame = checkNextAction();

    while (continueGame != QUIT)
    {
        if (continueGame == NEW_GAME)
        {
            newGame(word_length);
        }
        else if (continueGame == READ_WORDS)
        {
            fillDictionary(word_length);
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

    if (strcmp(input, "+nuova_partita") == 0)
        return NEW_GAME;
    if (strcmp(input, "+inserisci_inizio") == 0)
        return READ_WORDS;

    return QUIT;
}

void newGame(int word_length)
{
    user_knowledge *knownledge = getEmptyKnowledge();
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

        if (strcmp(inserted, "+stampa_filtrate") == 0)
        {
            list filtered = NULL;
            for (int i = 0; i < SIZE; i++)
            {
                char tmp[SIZE][S_LEN];
                int count = 0;

                if (table[i]->valid == '1')
                {
                    stpcpy(tmp[count], table[i]->str);
                    count++;
                }
            }
        }
        else if (strcmp(inserted, "+inserisci_inizio") == 0)
        {
            fillDictionary(word_length);
        }

        else if (strcmp(secret, inserted) == 0)
        {
            /// User win
            continue_game = 0;
            printf("ok\n");
        }
        else if (checkIfExixsts(inserted) == 0)
        {
            printf("not_exists\n");
        }
        else
        {
            attempts--;

            printHint(secret, inserted, word_length, knownledge);
            filterTable(knownledge);

            printf("%d\n", countValidItems());
        }

        if (attempts == 0)
        {
            printf("ko\n");
            continue_game = 0;
            clearKnowledge(knownledge);
            setAllStringAllValid();
            return;
        }
    }

    clearKnowledge(knownledge);
    setAllStringAllValid();

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

/// Return 1 if a new game should start, 0 otherwise
int fillDictionary(int length)
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
    stampaTabella();

            if (strcmp(string, "+nuova_partita") == 0)
                return 1;
            if (strcmp(string, "+inserisci_fine") == 0)
                return 0;
        }
        else if (strlen(string) != length)
        {
            printf("Errore nella lettura dell'input: La stringa fornita non rispetta il vincolo riguardo la lunghezza");
        }
        else
        {
            string[length] = '\0';

            table_item *item = malloc(sizeof(table_item));
            strcpy(item->str, string);
            item->valid = '1';

            int index = hash(string);

            while (table[index] != NULL)
            {

                index = (index + 1) % SIZE;
            }

            table[index] = item;
        }
    }

    return 0;
}

// TODO: rimuovi questo
void stampaTabella()
{
    for (int i = 0; i < SIZE; i++)
    {
        printf("%d: %s\n", i, table[i]->str);
    }
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

void sortAndPrintList(char items[][S_LEN], int count)
{
    quickSortMain(items, count);
    for (int i = 0; i < count; i++)
        printf("%s", items[i]);
}

void quickSortMain(char items[][S_LEN], int count)
{
    quickSort(items, 0, count - 1);
}

void quickSort(char items[][S_LEN], int left, int right)
{
    int i, j;
    char *x;
    char temp[S_LEN];

    i = left;
    j = right;
    x = items[(left + right) / 2];

    do
    {
        while ((strcmp(items[i], x) < 0) && (i < right))
        {
            i++;
        }
        while ((strcmp(items[j], x) > 0) && (j > left))
        {
            j--;
        }
        if (i <= j)
        {
            strcpy(temp, items[i]);
            strcpy(items[i], items[j]);
            strcpy(items[j], temp);
            i++;
            j--;
        }
    } while (i <= j);

    if (left < j)
    {
        quickSort(items, left, j);
    }
    if (i < right)
    {
        quickSort(items, i, right);
    }
}

int countValidItems()
{
    int counter = 0;

    for (int i = 0; i < SIZE; i++)
    {
        if (table[i] == NULL)
            continue;

        if (table[i]->valid == '1')
            counter++;
    }

    return counter;
}

void filterTable(user_knowledge *knowledge)
{
    for (int i = 0; i < SIZE; i++)
    {
        if (table[i] == NULL)
            continue;
        if (table[i]->valid == '0')
            continue;

        if (evaluateKnowledgeOnWord(table[i]->str, *knowledge) == 0)
            table[i]->valid = 0;
    }
}

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

int checkIfExixsts(char *string)
{
    for (int i = 0; i < SIZE; i++)
    {
        if (table[i] == NULL)
            continue;
        if (strcmp(table[i]->str, string) == 0)
            return 1;
    }

    return 1;
}

void setAllStringAllValid()
{
    //? Set all strings as valid
    for (int i = 0; i < SIZE; i++)
    {
        if (table[i] == NULL)
            continue;
        table[i]->valid = '1';
    }
}

/// Return 1 if the provided string contains at least one character
int stringContains(char *string, char character)
{
    for (int i = 0; string[i] != '\0'; i++)
        if (string[i] == character)
            return 1;
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