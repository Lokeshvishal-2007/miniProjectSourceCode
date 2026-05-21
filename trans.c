// Bank-account program with role-based login, password file, and record cleanup
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

// accountPassword structure definition
struct accountPassword
{
    unsigned int acctNum; // account number
    char password[20];    // password
}; // end structure accountPassword

// prototypes
void cleanCorruptedRecords(FILE *fPtr);
void createPasswordFile(void);
int authenticateUser(FILE *cfPtr, int *isAdmin, unsigned int *userAcctNum);
unsigned int enterChoice(int isAdmin);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void viewAccountDetails(FILE *fPtr, unsigned int accountNum);
void searchByName(FILE *fPtr);
void depositAmount(FILE *fPtr, unsigned int accountNum, int isUser);
void withdrawAmount(FILE *fPtr, unsigned int accountNum, int isUser);
void showTotalBalance(FILE *fPtr);
void viewTransactionLog(void);
void findMinMaxBalance(FILE *fPtr);
int isAllDigits(const char *text);
int isAllLetters(const char *text);
int readValidAccountNumber(unsigned int *accountNum, const char *prompt);
int readValidAmount(double *amount, const char *prompt);
void logTransaction(unsigned int accountNum, const char *type, double amount, double newBalance);

int main(int argc, char *argv[])
{
    FILE *cfPtr;                  // credit.dat file pointer
    unsigned int choice;          // user's choice
    int isAdmin;                  // flag for admin role
    unsigned int userAcctNum = 0; // for user login

    // fopen opens the file; exits if file cannot be opened
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    // clean corrupted records at startup
    cleanCorruptedRecords(cfPtr);

    // create password file if needed
    createPasswordFile();

    // authenticate user
    if (!authenticateUser(cfPtr, &isAdmin, &userAcctNum))
    {
        fclose(cfPtr);
        exit(-1);
    }

    // enable user to specify action
    while (1)
    {
        choice = enterChoice(isAdmin);

        if (isAdmin && choice == 11)
        {
            break; // admin logout
        }

        if (!isAdmin && choice == 5)
        {
            break; // user logout
        }

        if (isAdmin)
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
            // search record by name
            case 5:
                searchByName(cfPtr);
                break;
            // deposit amount into account
            case 6:
                depositAmount(cfPtr, 0, 0);
                break;
            // withdraw amount from account
            case 7:
                withdrawAmount(cfPtr, 0, 0);
                break;
            // show balance summary
            case 8:
                showTotalBalance(cfPtr);
                break;
            // display transaction log
            case 9:
                viewTransactionLog();
                break;
            // find highest and lowest balances
            case 10:
                findMinMaxBalance(cfPtr);
                break;
            // display if user does not select valid choice
            default:
                puts("Incorrect choice");
                break;
            } // end switch
        }
        else
        {
            // user menu
            switch (choice)
            {
            // view account details
            case 1:
                viewAccountDetails(cfPtr, userAcctNum);
                break;
            // deposit to my account
            case 2:
                depositAmount(cfPtr, userAcctNum, 1);
                break;
            // withdraw from my account
            case 3:
                withdrawAmount(cfPtr, userAcctNum, 1);
                break;
            // view my transaction log
            case 4:
                viewTransactionLog();
                break;
            // display if user does not select valid choice
            default:
                puts("Incorrect choice");
                break;
            } // end switch
        }
    } // end while

    fclose(cfPtr); // fclose closes the file
} // end main

// clean corrupted records from credit.dat
void cleanCorruptedRecords(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    struct clientData blankClient = {0, "", "", 0.0};
    int index;
    int cleanedCount = 0;

    rewind(fPtr);

    for (index = 0; index < 100; ++index)
    {
        if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
        {
            break;
        }

        // skip empty slots
        if (client.acctNum == 0)
        {
            continue;
        }

        // check corruption criteria
        if (client.acctNum < 1 || client.acctNum > 100 ||
            !isAllLetters(client.lastName) || !isAllLetters(client.firstName) ||
            client.balance < 0.0)
        {
            // overwrite with blank record
            fseek(fPtr, (index) * (long)sizeof(struct clientData), SEEK_SET);
            fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
            printf("Cleaned corrupted record at slot %d\n", index + 1);
            ++cleanedCount;
        }
    }

    printf("Total %d corrupted records cleaned\n", cleanedCount);
} // end function cleanCorruptedRecords

// create password file with default passwords
void createPasswordFile(void)
{
    FILE *pPtr;
    struct accountPassword accPass;
    unsigned int i;

    // check if password file exists
    if ((pPtr = fopen("passwords.dat", "rb")) != NULL)
    {
        fclose(pPtr);
        return; // file already exists
    }

    // create new password file
    if ((pPtr = fopen("passwords.dat", "wb")) == NULL)
    {
        puts("Could not create password file.");
        return;
    }

    // write default passwords for all accounts (1-100)
    for (i = 1; i <= 100; ++i)
    {
        accPass.acctNum = i;
        strcpy(accPass.password, "password123");
        fwrite(&accPass, sizeof(struct accountPassword), 1, pPtr);
    }

    fclose(pPtr);
    puts("Password file created with default passwords");
} // end function createPasswordFile

// authenticate user and determine role
int authenticateUser(FILE *cfPtr, int *isAdmin, unsigned int *userAcctNum)
{
    int loginChoice;
    int attempts;
    char username[32];
    char password[32];
    unsigned int accountNum;
    struct accountPassword accPass;
    struct clientData client;
    FILE *pPtr;

    puts("===== BANK MANAGEMENT SYSTEM =====");

    while (1)
    {
        printf("%s", "1 - Admin Login\n"
                     "2 - User Login\n"
                     "3 - Exit\n"
                     "Enter choice: ");
        if (scanf("%d", &loginChoice) != 1)
        {
            puts("Access Denied");
            return 0;
        }

        if (loginChoice == 1 || loginChoice == 2)
        {
            break;
        }

        if (loginChoice == 3)
        {
            exit(0);
        }

        puts("Incorrect choice");
    }

    if (loginChoice == 1)
    {
        // admin login
        for (attempts = 0; attempts < 3; ++attempts)
        {
            printf("%s", "Enter Username: ");
            if (scanf("%31s", username) != 1)
            {
                puts("Access Denied");
                return 0;
            }

            printf("%s", "Enter Password: ");
            if (scanf("%31s", password) != 1)
            {
                puts("Access Denied");
                return 0;
            }

            if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0)
            {
                puts("Access Granted");
                *isAdmin = 1;
                return 1;
            }

            if (attempts < 2)
            {
                printf("Incorrect username or password. %d attempts remaining\n", 2 - attempts);
            }
        }

        puts("Access Denied");
        return 0;
    }
    else
    {
        // user login
        for (attempts = 0; attempts < 3; ++attempts)
        {
            printf("%s", "Enter Account Number: ");
            if (!readValidAccountNumber(&accountNum, ""))
            {
                printf("Incorrect account. %d attempts remaining\n", 2 - attempts);
                continue;
            }

            printf("%s", "Enter Password: ");
            if (scanf("%31s", password) != 1)
            {
                printf("Incorrect password. %d attempts remaining\n", 2 - attempts);
                continue;
            }

            // open password file
            if ((pPtr = fopen("passwords.dat", "rb")) == NULL)
            {
                puts("Password file not found");
                return 0;
            }

            // search for account in password file
            int found = 0;
            while (fread(&accPass, sizeof(struct accountPassword), 1, pPtr) == 1)
            {
                if (accPass.acctNum == accountNum && strcmp(accPass.password, password) == 0)
                {
                    // password matches, get name from credit.dat
                    fseek(cfPtr, (accountNum - 1) * (long)sizeof(struct clientData), SEEK_SET);
                    fread(&client, sizeof(struct clientData), 1, cfPtr);

                    if (client.acctNum != 0)
                    {
                        printf("Welcome %s!\n", client.firstName);
                        *isAdmin = 0;
                        *userAcctNum = accountNum;
                        fclose(pPtr);
                        return 1;
                    }

                    found = 1;
                    break;
                }
            }

            fclose(pPtr);

            if (!found)
            {
                if (accPass.acctNum != accountNum)
                {
                    puts("Account not found");
                }
                else
                {
                    printf("Incorrect password. %d attempts remaining\n", 2 - attempts);
                }
            }
        }

        puts("Access Denied");
        return 0;
    }
} // end function authenticateUser

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
        fprintf(writePtr, "%-6s%-16s%-11s%-12s%10s\n", "Acct", "Last Name", "First Name", "Password", "Balance");

        // copy all records from random-access file into text file
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            // write single record to text file
            if (result != 0 && client.acctNum >= 1 && client.acctNum <= 100 && isAllLetters(client.lastName) && isAllLetters(client.firstName))
            {
                fprintf(writePtr, "%-6d%-16s%-11s%-12s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        "password123", client.balance);
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

// display a single account's details
void viewAccountDetails(FILE *fPtr, unsigned int accountNum)
{
    struct clientData client = {0, "", "", 0.0};

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
        return;
    }

    printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
} // end function viewAccountDetails

// search for an account by last name
void searchByName(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    char searchName[15];
    int found = 0;

    printf("%s", "Enter last name to search: ");
    if (scanf("%14s", searchName) != 1)
    {
        puts("Input ended unexpectedly.");
        return;
    }

    rewind(fPtr);

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0 && strcmp(client.lastName, searchName) == 0)
        {
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
            found = 1;
        }
    }

    if (!found)
    {
        puts("No match found");
    }
} // end function searchByName

// deposit money into an account
void depositAmount(FILE *fPtr, unsigned int accountNum, int isUser)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int acctToUse;
    double amount;

    if (isUser)
    {
        acctToUse = accountNum;
    }
    else
    {
        if (!readValidAccountNumber(&acctToUse, "Enter account number to deposit into ( 1 - 100 ): "))
        {
            puts("Invalid input. Could not read account number.");
            return;
        }
    }

    fseek(fPtr, (acctToUse - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", acctToUse);
        return;
    }

    if (!readValidAmount(&amount, "Enter deposit amount: "))
    {
        puts("Invalid input. Could not read amount.");
        return;
    }

    if (amount <= 0.0)
    {
        puts("Deposit amount must be greater than zero.");
        return;
    }

    client.balance += amount;
    printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

    fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    logTransaction(client.acctNum, "Deposit", amount, client.balance);
} // end function depositAmount

// withdraw money from an account
void withdrawAmount(FILE *fPtr, unsigned int accountNum, int isUser)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int acctToUse;
    double amount;

    if (isUser)
    {
        acctToUse = accountNum;
    }
    else
    {
        if (!readValidAccountNumber(&acctToUse, "Enter account number to withdraw from ( 1 - 100 ): "))
        {
            puts("Invalid input. Could not read account number.");
            return;
        }
    }

    fseek(fPtr, (acctToUse - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", acctToUse);
        return;
    }

    if (!readValidAmount(&amount, "Enter withdraw amount: "))
    {
        puts("Invalid input. Could not read amount.");
        return;
    }

    if (amount <= 0.0)
    {
        puts("Withdraw amount must be greater than zero.");
        return;
    }

    if (amount > client.balance)
    {
        puts("Insufficient funds");
        return;
    }

    client.balance -= amount;
    printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

    fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    logTransaction(client.acctNum, "Withdraw", amount, client.balance);
} // end function withdrawAmount

// show total number of active accounts and total balance
void showTotalBalance(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int activeAccounts = 0;
    double totalBalance = 0.0;
    int index;

    rewind(fPtr);

    for (index = 0; index < 100; ++index)
    {
        if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
        {
            break;
        }

        if (client.acctNum != 0)
        {
            ++activeAccounts;
            totalBalance += client.balance;
        }
    }

    printf("Total active accounts: %u\n", activeAccounts);
    printf("Total balance: %.2f\n", totalBalance);
} // end function showTotalBalance

// print all transaction log entries
void viewTransactionLog(void)
{
    FILE *logPtr;
    char line[256];

    if ((logPtr = fopen("trans_log.txt", "r")) == NULL)
    {
        puts("No transaction log found.");
        return;
    }

    while (fgets(line, sizeof(line), logPtr) != NULL)
    {
        printf("%s", line);
    }

    fclose(logPtr);
} // end function viewTransactionLog

// find highest and lowest balances among active accounts
void findMinMaxBalance(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    struct clientData minClient = {0, "", "", 0.0};
    struct clientData maxClient = {0, "", "", 0.0};
    int found = 0;
    int index;

    rewind(fPtr);

    for (index = 0; index < 100; ++index)
    {
        if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
        {
            break;
        }

        if (client.acctNum == 0)
        {
            continue;
        }

        if (!found)
        {
            minClient = client;
            maxClient = client;
            found = 1;
        }
        else
        {
            if (client.balance < minClient.balance)
            {
                minClient = client;
            }

            if (client.balance > maxClient.balance)
            {
                maxClient = client;
            }
        }
    }

    if (!found)
    {
        puts("No active accounts found.");
        return;
    }

    puts("Highest balance account:");
    printf("%-6d%-16s%-11s%10.2f\n", maxClient.acctNum, maxClient.lastName, maxClient.firstName, maxClient.balance);
    puts("Lowest balance account:");
    printf("%-6d%-16s%-11s%10.2f\n", minClient.acctNum, minClient.lastName, minClient.firstName, minClient.balance);
} // end function findMinMaxBalance

void logTransaction(unsigned int accountNum, const char *type, double amount, double newBalance)
{
    FILE *logPtr;

    if ((logPtr = fopen("trans_log.txt", "a")) == NULL)
    {
        puts("File could not be opened.");
        return;
    }

    fprintf(logPtr, "Acct %u | %s: %.2f | New Balance: %.2f\n", accountNum, type, amount, newBalance);
    fclose(logPtr);
}

int readValidAmount(double *amount, const char *prompt)
{
    char amountInput[50];
    char *endPtr;

    if (amount == NULL)
    {
        return 0;
    }

    while (1)
    {
        printf("%s", prompt);
        if (scanf("%49s", amountInput) != 1)
        {
            return 0;
        }

        *amount = strtod(amountInput, &endPtr);
        if (endPtr != amountInput && *endPtr == '\0')
        {
            return 1;
        }

        puts("Error: amount should be a valid number.");
    }
}

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
unsigned int enterChoice(int isAdmin)
{
    unsigned int menuChoice; // variable to store user's choice

    if (isAdmin)
    {
        printf("%s", "\nEnter your choice\n"
                     "1 - Print all accounts to accounts.txt\n"
                     "2 - Update account balance\n"
                     "3 - Add new account\n"
                     "4 - Delete account\n"
                     "5 - Search account by name\n"
                     "6 - Deposit\n"
                     "7 - Withdraw\n"
                     "8 - Show total balance summary\n"
                     "9 - View transaction log\n"
                     "10 - Find highest and lowest balance\n"
                     "11 - Logout\n? ");
    }
    else
    {
        printf("%s", "\nEnter your choice\n"
                     "1 - View my account details\n"
                     "2 - Deposit to my account\n"
                     "3 - Withdraw from my account\n"
                     "4 - View my transaction log\n"
                     "5 - Logout\n? ");
    }

    scanf("%u", &menuChoice); // receive choice from user
    return menuChoice;
} // end function enterChoice