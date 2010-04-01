#include "../interface/GenJetAnalyzer.h"

using namespace std;
using namespace reco;
using namespace edm;

GenJetAnalyzer::GenJetAnalyzer(const edm::ParameterSet& producersNames):verbosity_(0)
{
	dataType_ = producersNames.getUntrackedParameter<string>("dataType","unknown");
	genJetProducer_ = producersNames.getParameter<edm::InputTag>("genJetProducer");
	myJetAnalyzer = new JetAnalyzer();
}

GenJetAnalyzer::GenJetAnalyzer(const edm::ParameterSet& producersNames, int verbosity):verbosity_(verbosity)
{
	dataType_ = producersNames.getUntrackedParameter<string>("dataType","unknown");
	genJetProducer_ = producersNames.getParameter<edm::InputTag>("genJetProducer");
	myJetAnalyzer = new JetAnalyzer(verbosity);
}

GenJetAnalyzer::GenJetAnalyzer(const edm::ParameterSet& producersNames, const edm::ParameterSet& myConfig, int verbosity):verbosity_(verbosity)
{
	dataType_ = producersNames.getUntrackedParameter<string>("dataType","unknown");
	genJetProducer_ = producersNames.getParameter<edm::InputTag>("genJetProducer");
	myJetAnalyzer = new JetAnalyzer(myConfig, verbosity);
}

GenJetAnalyzer::GenJetAnalyzer(const edm::ParameterSet& producersNames, int iter, const edm::ParameterSet& myConfig, int verbosity):verbosity_(verbosity)
{
	dataType_ = producersNames.getUntrackedParameter<string>("dataType","unknown");
	vGenJetProducer = producersNames.getUntrackedParameter<std::vector<std::string> >("vcaloJetProducer");
	genJetProducer_ = edm::InputTag(vGenJetProducer[iter]);
	myJetAnalyzer = new JetAnalyzer(myConfig, verbosity);
}

GenJetAnalyzer::~GenJetAnalyzer()
{
}

void GenJetAnalyzer::Process(const edm::Event& iEvent, TClonesArray* rootGenJets)
{

	unsigned int nJets=0;

	// check if the genJet is of the good type
	std::string jetType = "BASIC";
	if( genJetProducer_.label()=="kt4GenJets"
		|| genJetProducer_.label()=="kt6GenJets"
		|| genJetProducer_.label()=="iterativeCone5GenJets"
		|| genJetProducer_.label()=="sisCone5GenJets"
		|| genJetProducer_.label()=="sisCone7GenJets"
      || genJetProducer_.label()=="ak5GenJets"
      || genJetProducer_.label()=="ak7GenJets"
	) jetType="CALO";

	edm::Handle < edm::View <reco::GenJet> > recoGenJets;
	if( (dataType_=="RECO" || dataType_=="AOD" || dataType_=="PAT" || dataType_=="PATAOD") && jetType=="CALO" )
	{
		iEvent.getByLabel(genJetProducer_, recoGenJets);
		nJets = recoGenJets->size();
	}

			
	if(verbosity_>1) std::cout << "   Number of jets = " << nJets << "   Label: " << genJetProducer_.label() << "   Instance: " << genJetProducer_.instance() << std::endl;

	for (unsigned int j=0; j<nJets; j++)
	{
		const reco::GenJet* genJet = 0;	
		if( (dataType_=="RECO" || dataType_=="AOD" || dataType_=="PAT" || dataType_=="PATAOD") && jetType=="CALO" ) genJet = (const reco::GenJet*) ( & ((*recoGenJets)[j]) );
			
		// Call JetAnalyzer to fill the basic Jet Properties
		TRootJet tempJet = (TRootJet) myJetAnalyzer->Process( &( *(genJet) ), dataType_);

		TRootGenJet localGenJet = TRootGenJet(tempJet);

		localGenJet.setJetType(1); // 1 = CaloJet
		localGenJet.setN90(genJet->nCarrying(0.9));
		localGenJet.setN60(genJet->nCarrying(0.6));
		localGenJet.setetaetaMoment(genJet->etaetaMoment());
		localGenJet.setphiphiMoment(genJet->phiphiMoment());
		localGenJet.setEcalEnergy(genJet->emEnergy());
		localGenJet.setHcalEnergy(genJet->hadEnergy());

				
		new( (*rootGenJets)[j] ) TRootGenJet(localGenJet);
		if(verbosity_>2) cout << "   ["<< setw(3) << j << "] " << localGenJet << endl;
		
	}

}