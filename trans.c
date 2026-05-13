// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
}; // end structure clientData

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
int isAllDigits(const char *text);
int isAllLetters(const char *text);
int readValidAccountNumber(unsigned int *accountNum, const char *prompt);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; exits if file cannot be opened
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 5)
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            textFile(cfPtr);
            break;
        // update record
        case 2:
            updateRecord(cfPtr);
            break;
        // create record
        case 3:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 4:
            deleteRecord(cfPtr);
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    } // end while

    fclose(cfPtr); // fclose closes the file
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            // write single record to text file
            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
            } // end if
        } // end while

        fclose(writePtr); // fclose closes the file
    } // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    // create clientData with no information
    struct clientData client = {0, "", "", 0.0};

    // obtain number of account to update
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%d", &account);

    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    { // update record
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        client.balance += transaction; // update record balance

        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // move file pointer to correct record in file
        // move back by 1 record length
        fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", 0}; // blank client
    unsigned int accountNum;                        // account number

    // obtain number of account to delete
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    } // end if
    else
    { // delete record
        // move file pointer to correct record in file
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        // replace existing record with blank record
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum; // account number
    char balanceInput[50];
    char *endPtr;

    // obtain number of account to create
    if (!readValidAccountNumber(&accountNum, "Enter new account number ( 1 - 100 ): "))
    {
        puts("Invalid input. Could not read account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name with letters only
        while (1)
        {
            printf("%s", "Enter last name (letters only): ");
            if (scanf("%14s", client.lastName) != 1)
            {
                puts("Input ended unexpectedly.");
                return;
            }

            if (isAllLetters(client.lastName))
            {
                break;
            }

            puts("Error: last name should contain only letters.");
        }

        // user enters first name with letters only
        while (1)
        {
            printf("%s", "Enter first name (letters only): ");
            if (scanf("%9s", client.firstName) != 1)
            {
                puts("Input ended unexpectedly.");
                return;
            }

            if (isAllLetters(client.firstName))
            {
                break;
            }

            puts("Error: first name should contain only letters.");
        }

        // user enters numeric balance
        while (1)
        {
            printf("%s", "Enter balance: ");
            if (scanf("%49s", balanceInput) != 1)
            {
                puts("Input ended unexpectedly.");
                return;
            }

            client.balance = strtod(balanceInput, &endPtr);
            if (endPtr != balanceInput && *endPtr == '\0')
            {
                break;
            }

            puts("Error: balance should be a valid number.");
        }

        client.acctNum = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function newRecord

int isAllDigits(const char *text)
{
    size_t index;

    if (text == NULL || text[0] == '\0')
    {
        return 0;
    }

    for (index = 0; text[index] != '\0'; ++index)
    {
        if (!isdigit((unsigned char)text[index]))
        {
            return 0;
        }
    }

    return 1;
}

int isAllLetters(const char *text)
{
    size_t index;

    if (text == NULL || text[0] == '\0')
    {
        return 0;
    }

    for (index = 0; text[index] != '\0'; ++index)
    {
        if (!isalpha((unsigned char)text[index]))
        {
            return 0;
        }
    }

    return 1;
}

int readValidAccountNumber(unsigned int *accountNum, const char *prompt)
{
    char accountInput[30];
    unsigned long parsedValue;

    if (accountNum == NULL)
    {
        return 0;
    }

    while (1)
    {
        printf("%s", prompt);
        if (scanf("%29s", accountInput) != 1)
        {
            return 0;
        }

        if (!isAllDigits(accountInput))
        {
            puts("Error: account number should contain only digits.");
            continue;
        }

        parsedValue = strtoul(accountInput, NULL, 10);
        if (parsedValue < 1 || parsedValue > 100)
        {
            puts("Error: account number must be between 1 and 100.");
            continue;
        }

        *accountNum = (unsigned int)parsedValue;
        return 1;
    }
}

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - end program\n? ");

    scanf("%u", &menuChoice); // receive choice from user
    return menuChoice;
} // end function enterChoice