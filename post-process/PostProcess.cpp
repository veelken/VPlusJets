#include <cstdio>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH1I.h"

using namespace std;

class CrossSection{
public:
	int ReadTxtFile(const char*fileName);
	const static int noFile=-1;
	const static double noMatch=-2;
	const static double multipleMatch=-3;
	double xSection(string match);
private:
	map<string,double> xSec;	
};
int CrossSection::ReadTxtFile(const char*fileName)
	{
	FILE *fr=fopen(fileName,"r");
	if(fr==NULL) return CrossSection::noFile;
	char key[1023];
	double number;
	while(fscanf(fr,"%s %lf",key,&number)!=EOF){
		xSec[string(key)]=number;
		}
	return 0;
	}
double CrossSection::xSection(string match){
	float R=0;
	int m=0;
	for(map<string,double>::iterator it=xSec.begin();it!=xSec.end();it++)
		{
		if(match.find(it->first) != string::npos){ // I want to match what I put in the database and not the fileName
			R=it->second;
			m++;
			//DEBUG
			fprintf(stderr,"MATCH=%s\n",it->first.c_str());
			}	
		}
	if(m==1) return R;
	else if( m==0) return CrossSection::noMatch;
	else if( m>1) return CrossSection::multipleMatch;
	}

#include "AddEventWeight.C"
#include "AddPUWeight.C"
#include "AddRDWeight.C"

int Usage(const char *progName){
	printf("Usage:\n");
	printf("      %s fileName [pu]\n",progName);
	printf("             - pu 0: add Cross Section Weights  And PUWeight. Pileupfile is pileup/all.json.pileup.root\n");
	printf("             - pu 1: add PUWeight+1s. Pileupfile is pileup/all.json.pileup_UP.root\n");
	printf("             - pu -1: add PUWeight-1s. Pileupfile is pileup/all.json.pileup_DN.root\n");
	printf("             - pu 5: Do pu 0 + 1 + -1 + HLT PUWeights\n");
	printf("             - pu 6: HLT PUWeights\n");
	printf("             - pu 10: add PUWeight \n");
	printf("             - pu 100: add RDWeights \n");
	return 0;
}

int main(int argc, char**argv){
	if(argc<2){return Usage(argv[0]);}
	if(argc==2){
		printf("2\n");//DEBUG
		CrossSection A;
			size_t found;
			found=string(argv[0]).find_last_of("/\\");	
			string folder=string(argv[0]).substr(0,found);
		A.ReadTxtFile( (folder+ "/xSec.ini").c_str());
		double xSec=A.xSection(argv[1]);
		if (xSec==CrossSection::noMatch){printf("No match Cross Section in the Database\n");return -1;}
		if (xSec==CrossSection::multipleMatch){printf("Multiple matches in Cross Section in the Database\n");return -1;}
		printf("xSec=%.1lf\n",xSec);
		return AddEventWeight(argv[1],"accepted",xSec,"events","WEvents",1.0);
		}
	if(argc==3){
		int pu;
		printf("3\n");//DEBUG
		sscanf(argv[2],"%d",&pu);
		if(pu==0 || pu==5){
			CrossSection A;
				size_t found;
				found=string(argv[0]).find_last_of("/\\");	
				string folder=string(argv[0]).substr(0,found);
			A.ReadTxtFile( (folder+ "/xSec.ini").c_str());
			double xSec=A.xSection(argv[1]);
			if (xSec==CrossSection::noMatch){printf("No match Cross Section in the Database\n");return -1;}
			if (xSec==CrossSection::multipleMatch){printf("Multiple matches in Cross Section in the Database\n");return -1;}
			printf("xSec=%.1lf\n",xSec);
			AddEventWeight(argv[1],"accepted",xSec,"events","WEvents",1.0);
			AddPUWeight(argv[1],"mcTruePU");
		}else if(pu==10){
			AddPUWeight(argv[1],"mcTruePU");
		}else if(pu ==1){
			AddPUWeight(argv[1],"mcTruePU","accepted","events",1,1,"pileup/all.json.pileup_UP.root");
		}else if(pu==-1){
			AddPUWeight(argv[1],"mcTruePU","accepted","events",-1,1,"pileup/all.json.pileup_DN.root");
			}
		else if(pu==100)
			{
			CrossSection A;
				size_t found;
				found=string(argv[0]).find_last_of("/\\");	
				string folder=string(argv[0]).substr(0,found);
			A.ReadTxtFile( (folder+ "/xSec.ini").c_str());
			double xSec=A.xSection(argv[1]);

			if (xSec==CrossSection::noMatch){printf("No match Cross Section in the Database\n");return -1;}
			if (xSec==CrossSection::multipleMatch){printf("Multiple matches in Cross Section in the Database\n");return -1;}
			printf("xSec=%.1lf\n",xSec);
	
			printf("Going to execute:\n AddRDWeight.C+'(\"%s\",\"accepted\",\"events\",%f,\"processedData\",\"%s/RunAndLumi.txt\",1.0)';\n",argv[1],xSec,folder.c_str());	
			AddRDWeight(argv[1],"accepted","events",xSec,"processedData",string(folder + "/RunAndLumi.txt").c_str(),1.0);
			}
		if(pu == 5 ) {
			AddPUWeight(argv[1],"mcTruePU","accepted","events",1,1,"pileup/all.json.pileup_UP.root");
			AddPUWeight(argv[1],"mcTruePU","accepted","events",-1,1,"pileup/all.json.pileup_DN.root");
			}
		if( pu==5 || pu ==6){
			printf("Going to do HLT150\n");
			AddPUWeight(argv[1],"mcTruePU","accepted","events",0 ,1,"pileup/MyPileup_HLT_Photon150_v.root","PUWeight_HLT_Photon150"); // the Branch Name for Syst is fixed inside
			AddPUWeight(argv[1],"mcTruePU","accepted","events",1 ,1,"pileup/MyPileup_UP_HLT_Photon150_v.root","PUWeight_HLT_Photon150"); 
			AddPUWeight(argv[1],"mcTruePU","accepted","events",-1,1,"pileup/MyPileup_DN_HLT_Photon150_v.root","PUWeight_HLT_Photon150");

			printf("Going to do HLT135\n");
			AddPUWeight(argv[1],"mcTruePU","accepted","events",0 ,1,"pileup/MyPileup_HLT_Photon135_v.root","PUWeight_HLT_Photon135"); // the Branch Name for Syst is fixed inside
			AddPUWeight(argv[1],"mcTruePU","accepted","events",1 ,1,"pileup/MyPileup_UP_HLT_Photon135_v.root","PUWeight_HLT_Photon135"); 
			AddPUWeight(argv[1],"mcTruePU","accepted","events",-1,1,"pileup/MyPileup_DN_HLT_Photon135_v.root","PUWeight_HLT_Photon135");

			printf("Going to do HLT90\n");
			AddPUWeight(argv[1],"mcTruePU","accepted","events",0 ,1,"pileup/MyPileup_HLT_Photon90_CaloIdVL_v.root","PUWeight_HLT_Photon90"); //
			AddPUWeight(argv[1],"mcTruePU","accepted","events",1 ,1,"pileup/MyPileup_UP_HLT_Photon90_CaloIdVL_v.root","PUWeight_HLT_Photon90"); 
			AddPUWeight(argv[1],"mcTruePU","accepted","events",-1,1,"pileup/MyPileup_DN_HLT_Photon90_CaloIdVL_v.root","PUWeight_HLT_Photon90");

			printf("Going to do HLT75\n");
			AddPUWeight(argv[1],"mcTruePU","accepted","events",0 ,1,"pileup/MyPileup_HLT_Photon75_CaloIdVL_v.root","PUWeight_HLT_Photon75"); //
			AddPUWeight(argv[1],"mcTruePU","accepted","events",1 ,1,"pileup/MyPileup_UP_HLT_Photon75_CaloIdVL_v.root","PUWeight_HLT_Photon75"); 
			AddPUWeight(argv[1],"mcTruePU","accepted","events",-1,1,"pileup/MyPileup_DN_HLT_Photon75_CaloIdVL_v.root","PUWeight_HLT_Photon75");

			printf("Going to do HLT50\n");
			AddPUWeight(argv[1],"mcTruePU","accepted","events",0 ,1,"pileup/MyPileup_HLT_Photon50_CaloIdVL_v.root","PUWeight_HLT_Photon50"); //
			AddPUWeight(argv[1],"mcTruePU","accepted","events",1 ,1,"pileup/MyPileup_UP_HLT_Photon50_CaloIdVL_v.root","PUWeight_HLT_Photon50"); 
			AddPUWeight(argv[1],"mcTruePU","accepted","events",-1,1,"pileup/MyPileup_DN_HLT_Photon50_CaloIdVL_v.root","PUWeight_HLT_Photon50");

			printf("Going to do HLT30\n");
			AddPUWeight(argv[1],"mcTruePU","accepted","events",0 ,1,"pileup/MyPileup_HLT_Photon30_CaloIdVL_v.root","PUWeight_HLT_Photon30"); //
			AddPUWeight(argv[1],"mcTruePU","accepted","events",1 ,1,"pileup/MyPileup_UP_HLT_Photon30_CaloIdVL_v.root","PUWeight_HLT_Photon30"); 
			AddPUWeight(argv[1],"mcTruePU","accepted","events",-1,1,"pileup/MyPileup_DN_HLT_Photon30_CaloIdVL_v.root","PUWeight_HLT_Photon30");
			}
				
			
		
		}

}
