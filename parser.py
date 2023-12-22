from bs4 import BeautifulSoup
import sys


def getCompanyNameAndStockPriceAndWriteToFile(inputFileName, outputFileName):
    appendPath = "http_dump/dump_http/files/"
    companyName = ""
    companyPriceName = ""
    inputFileName = appendPath + inputFileName

    # Reading the values form file
    with open(inputFileName, 'r') as inputFile:
        html_doc = inputFile.read()

        # In BeautifulSoup, to parse and get relevant data
        soup = BeautifulSoup(html_doc, 'html.parser')

        divWithData = soup.find('div', id='__next')
        if(divWithData is None) :
            companyName = inputFileName
            companyPriceName = "0.0"
        else :
            # for name
            companyName = divWithData.find('h1').text
            # for price
            companyPriceName = divWithData.find('div', class_='text-5xl/9 font-bold md:text-[42px] md:leading-[60px] text-[#232526]').text

    outPutForFile = ""
    outPutForFile +=  companyName + "="
    outPutForFile += companyPriceName
    with open(outputFileName, 'w') as output_file:
        output_file.write(outPutForFile)


# Taking input from command line
if(len(sys.argv) != 3):
    print("Input Format: python3 <input_file> <output_file>")
    sys.exit("Kindly try again with correct input")
inputFileName = str(sys.argv[1])
outputFileName = str(sys.argv[2])


getCompanyNameAndStockPriceAndWriteToFile(inputFileName, outputFileName)