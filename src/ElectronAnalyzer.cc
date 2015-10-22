#include "../interface/ElectronAnalyzer.h"

using namespace std;
using namespace TopTree;
using namespace reco;
using namespace edm;
using namespace isodeposit;

ElectronAnalyzer::ElectronAnalyzer(const edm::ParameterSet& producersNames):verbosity_(0),useMC_(false),runSuperCluster_(false),doPrimaryVertex_(false),isData_(false)

{
  electronProducer_ = producersNames.getParameter<edm::InputTag>("electronProducer");
  primaryVertexProducer_ = producersNames.getParameter<edm::InputTag>("primaryVertexProducer");
  TrackLabel_ = producersNames.getParameter<edm::InputTag>("generalTrackLabel");
  newId_ = producersNames.getUntrackedParameter<bool>("electronNewId",false);
}

ElectronAnalyzer::ElectronAnalyzer(const edm::ParameterSet& producersNames, const edm::ParameterSet& myConfig, int verbosity):verbosity_(verbosity)
{
  electronProducer_ = producersNames.getParameter<edm::InputTag>("electronProducer");
  useMC_ = myConfig.getUntrackedParameter<bool>("doElectronMC");
  isData_ = myConfig.getUntrackedParameter<bool>("isData");
  runSuperCluster_ = myConfig.getUntrackedParameter<bool>("runSuperCluster",false);
  primaryVertexProducer_ = producersNames.getParameter<edm::InputTag>("primaryVertexProducer");
  doPrimaryVertex_ = myConfig.getUntrackedParameter<bool>("doPrimaryVertex");
  TrackLabel_ = producersNames.getParameter<edm::InputTag>("generalTrackLabel");
  newId_ = producersNames.getUntrackedParameter<bool>("electronNewId",false);
}

ElectronAnalyzer::ElectronAnalyzer(const edm::ParameterSet& producersNames, int iter, const edm::ParameterSet& myConfig, int verbosity):verbosity_(verbosity)
{
  vElectronProducer = producersNames.getUntrackedParameter<std::vector<std::string> >("velectronProducer");
  electronProducer_ =	edm::InputTag(vElectronProducer[iter]);
  useMC_ = myConfig.getUntrackedParameter<bool>("doElectronMC");
  isData_ = myConfig.getUntrackedParameter<bool>("isData");
  runSuperCluster_ = myConfig.getUntrackedParameter<bool>("runSuperCluster",false);
  primaryVertexProducer_ = producersNames.getParameter<edm::InputTag>("primaryVertexProducer");
  doPrimaryVertex_ = myConfig.getUntrackedParameter<bool>("doPrimaryVertex");
  TrackLabel_ = producersNames.getParameter<edm::InputTag>("generalTrackLabel");
  newId_ = producersNames.getUntrackedParameter<bool>("electronNewId",false);
}

ElectronAnalyzer::~ElectronAnalyzer()
{
}

void ElectronAnalyzer::Process(const edm::Event& iEvent, TClonesArray* rootElectrons, const edm::EventSetup& iSetup )
{
  unsigned int nElectrons=0;

  edm::Handle < std::vector <pat::Electron> > patElectrons;
  iEvent.getByLabel(electronProducer_, patElectrons);
  nElectrons = patElectrons->size();

  edm::Handle< reco::VertexCollection > pvHandle;
  iEvent.getByLabel(primaryVertexProducer_, pvHandle);

  edm::Handle<reco::BeamSpot> beamSpotHandle;
  iEvent.getByLabel("offlineBeamSpot", beamSpotHandle);

  //edm::ESHandle<TransientTrackBuilder> builder;
  //iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder", builder);
  //TransientTrackBuilder thebuilder = *(builder.product());

  //edm::Handle<reco::BeamSpot> beamSpotHandle;
  //iEvent.getByLabel("offlineBeamSpot", beamSpotHandle);
  //const reco::TrackBase::Point & beamSpot = reco::TrackBase::Point(beamSpotHandle->x0(), beamSpotHandle->y0(), beamSpotHandle->z0());

  if(verbosity_>1) std::cout << "   Number of electrons = " << nElectrons << "   Label: " << electronProducer_.label() << "   Instance: " << electronProducer_.instance() << std::endl;

  for (unsigned int j=0; j<nElectrons; j++)
  {
    const pat::Electron*  patElectron = &((*patElectrons)[j]);//dynamic_cast<const pat::Electron*>(&*electron);
    const reco::GsfElectron* electron = (const reco::GsfElectron*) patElectron;//( & ((*patElectrons)[j]) );

    TRootElectron localElectron(
                                electron->px()
                                ,electron->py()
                                ,electron->pz()
                                ,electron->energy()
                                ,electron->vx()
                                ,electron->vy()
                                ,electron->vz()
                                ,electron->pdgId()
                                ,electron->charge()
                                );

    //cout<<"in electronAnalyzer...1"<<endl;
    //=======================================
    localElectron.setEcalSeeding(electron->ecalDrivenSeed());
    localElectron.setTrackerSeeding(electron->trackerDrivenSeed());
    localElectron.setEnergySuperClusterOverP(electron->eSuperClusterOverP());
    localElectron.setEnergyEleClusterOverPout(electron->eEleClusterOverPout());
    localElectron.setEnergySeedClusterOverPout(electron->eSeedClusterOverPout());

    localElectron.setDeltaEtaIn(electron->deltaEtaSuperClusterTrackAtVtx());
    localElectron.setDeltaEtaOut(electron->deltaEtaSeedClusterTrackAtCalo());
    localElectron.setDeltaPhiIn(electron->deltaPhiSuperClusterTrackAtVtx());
    localElectron.setDeltaPhiOut(electron->deltaPhiSeedClusterTrackAtCalo());
    localElectron.setDeltaPhiSuperClusterTrackAtCalo(electron->deltaPhiEleClusterTrackAtCalo());
    localElectron.setDeltaEtaSuperClusterTrackAtCalo(electron->deltaEtaEleClusterTrackAtCalo());

    localElectron.setFbrem(electron->fbrem());
    localElectron.setNBrems(electron->numberOfBrems());

    //setShowerShape
    localElectron.setE1x5( electron->e1x5() );
    localElectron.setE5x5( electron->e5x5() );
    localElectron.setHoverEDepth1( electron->hcalDepth1OverEcal() );
    localElectron.setHoverEDepth2( electron->hcalDepth2OverEcal() );
    localElectron.setSigmaIetaIeta_full5x5( electron->full5x5_sigmaIetaIeta() );
    localElectron.setSigmaIetaIeta( electron->sigmaIetaIeta() );

    // switching to gsf track for miniAOD, seems this is the correct thing for
    //now
    //    reco::TrackRef myTrackRef = electron->closestCtfTrackRef();
    reco::GsfTrackRef myTrackRef = electron->gsfTrack();

   if ( myTrackRef.isNonnull() )
    {
	    const reco::HitPattern& hit = myTrackRef->hitPattern();
	    localElectron.setPixelLayersWithMeasurement(hit.pixelLayersWithMeasurement());
	    localElectron.setStripLayersWithMeasurement(hit.stripLayersWithMeasurement());
      localElectron.setTrackNormalizedChi2(myTrackRef->normalizedChi2());
      localElectron.setNValidHits(myTrackRef->numberOfValidHits());
    }

    reco::GsfTrackRef gsfTrack = electron->gsfTrack();

    if ( gsfTrack.isNonnull() )
    {
      localElectron.setGsfTrackNormalizedChi2(gsfTrack->normalizedChi2());
      localElectron.setTrackMissingHits(gsfTrack->numberOfLostHits());
      localElectron.setGsfTrackChi2(gsfTrack->chi2());
      localElectron.setGsfTrackNdof(gsfTrack->ndof());

      if(doPrimaryVertex_ && pvHandle.isValid() && pvHandle->size() != 0){
        reco::VertexRef vtx(pvHandle, 0);
        localElectron.setD0( gsfTrack->dxy(vtx->position()) );
        localElectron.setD0Error( sqrt( pow(gsfTrack->dxyError(),2) + pow(vtx->xError(),2) + pow(vtx->yError(),2) ) );
  	localElectron.setDz( gsfTrack->dz(vtx->position()) );
	localElectron.setDzError( gsfTrack->dzError());


	// get the beam spot and use that to get relative position. Used in displaced lepton analysis 
	const reco::BeamSpot &mybeamspot = *beamSpotHandle.product();
	localElectron.setD0BeamSpot(patElectron->gsfTrack()->dxy(mybeamspot.position()));
        localElectron.setD0BeamSpotError( sqrt( pow(patElectron->gsfTrack()->dxyError(),2) + pow(mybeamspot.x0Error(),2) + pow(mybeamspot.y0Error(),2) ) );
	localElectron.setDzBeamSpot(patElectron->gsfTrack()->dz(mybeamspot.position()));
	localElectron.setDzBeamSpotError( sqrt( pow(patElectron->gsfTrack()->dzError(),2) + pow(mybeamspot.x0Error(),2) + pow(mybeamspot.y0Error(),2) ) );


        //do we really need to ip3d? we can use PAT member function instead of getting it from RECO (taejeong)
        //const double gsfsign   = ( (-gsfTrack->dxy(vtx->position()))   >=0 ) ? 1. : -1.;
        //const reco::TransientTrack &tt = thebuilder.build(gsfTrack);
        //const std::pair<bool,Measurement1D> &ip3dpv =  IPTools::absoluteImpactParameter3D(tt,((*pvHandle)[j]));
        //if (ip3dpv.first)
        //{
	//        localElectron.setIp3d(gsfsign*ip3dpv.second.value());
	//        localElectron.setIp3dError(ip3dpv.second.error());
  	//}
        localElectron.setIp3d( patElectron->ip3d() );

      }
    }
    else if ( myTrackRef.isNonnull() )
    {

      if(doPrimaryVertex_ && pvHandle.isValid() && pvHandle->size() != 0)
	    {
	      reco::VertexRef vtx(pvHandle, 0);
        localElectron.setD0( myTrackRef->dxy(vtx->position()) );
        localElectron.setD0Error( sqrt( pow(myTrackRef->dxyError(),2) + pow(vtx->xError(),2) + pow(vtx->yError(),2) ) );
  	    localElectron.setDz( myTrackRef->dz(vtx->position()) );
      }
    }

    reco::SuperClusterRef superCluster = electron->superCluster();
    if ( superCluster.isNonnull() && runSuperCluster_ )
	  {
	    localElectron.setSuperClusterRawEnergy(superCluster->rawEnergy());
	    localElectron.setSuperClusterEta(superCluster->eta());
	    localElectron.setPreshowerEnergy(superCluster->preshowerEnergy());
      localElectron.setIoEmIoP( (1./superCluster->energy()) - (1./electron->p()) );
      if ( gsfTrack.isNonnull() ) localElectron.setIoEmIoPgsf( (1./superCluster->energy()) - (1./gsfTrack->p()) );
      localElectron.setEtaWidth( superCluster->etaWidth() );
      localElectron.setPhiWidth( superCluster->phiWidth() );
	  }

    //Isolation
    //  localElectron.setIsoR03_hcalIso(electron->dr03HcalTowerSumEt());
    // localElectron.setIsoR03_ecalIso(electron->dr03EcalRecHitSumEt());
    //localElectron.setIsoR03_trackIso(electron->dr03TkSumPt());

    //localElectron.setIsoR04_hcalIso(electron->dr04HcalTowerSumEt());
    // localElectron.setIsoR04_ecalIso(electron->dr04EcalRecHitSumEt());
    //    localElectron.setIsoR04_trackIso(electron->dr04TkSumPt());


    // Conversion:
    // we don't use this coversion anymore (taejeong)
    /*
    double evt_bField = 0; // need the magnetic field
    // if isData_ then derive bfield using the magnet current from DcsStatus otherwise take it from the IdealMagneticFieldRecord

    if (!isData_)
    {
    	ESHandle<MagneticField> magneticField;
	    iSetup.get<IdealMagneticFieldRecord>().get(magneticField);
	    evt_bField = magneticField->inTesla(GlobalPoint(0.,0.,0.)).z();
    }
    else
    {
	    edm::Handle<DcsStatusCollection> dcsHandle;
	    iEvent.getByLabel("scalersRawToDigi", dcsHandle);
	    // scale factor = 3.801/18166.0 which are average values taken over a stable two week period
    	float currentToBFieldScaleFactor = 2.09237036221512717e-04;
	    float current = (*dcsHandle)[0].magnetCurrent();
	    evt_bField = current*currentToBFieldScaleFactor;
    }
    edm::Handle<reco::TrackCollection> tracks_h;
    iEvent.getByLabel(TrackLabel_, tracks_h);

    ConversionFinder convFinder;
    const ConversionInfo convInfo = convFinder.getConversionInfo(*electron, tracks_h, evt_bField, 0.45);
    localElectron.setDist(convInfo.dist());
    localElectron.setDCot(convInfo.dcot());
    */

    // Some specific methods to pat::Electron
    TLorentzVector ecalDrivenMomentum(patElectron->ecalDrivenMomentum().px(),patElectron->ecalDrivenMomentum().py(),patElectron->ecalDrivenMomentum().pz(),patElectron->ecalDrivenMomentum().energy());
    localElectron.setEcalDrivenMomentum(ecalDrivenMomentum);
<<<<<<< HEAD

    //    localElectron.setdB(patElectron->dB());
=======
    localElectron.setIsPF(patElectron->isPF());
    //localElectron.setdB(patElectron->dB());
>>>>>>> e06eb322df2e4b6d2bbed7be4e34f11b7c176c64
    //localElectron.setdBError(patElectron->edB());

    // Matched genParticle
    if(useMC_)
	  {
	    if ((patElectron->genParticleRef()).isNonnull()) localElectron.setGenParticleIndex((patElectron->genParticleRef()).index());
	    else localElectron.setGenParticleIndex(-1);
	  }

    //set pf-isolation
    //default cone size from PAT setup : currently r=0.3
    if(patElectron->chargedHadronIso() != -1) localElectron.setIsoR03_ChargedHadronIso(patElectron->chargedHadronIso());
    if(patElectron->puChargedHadronIso() != -1) localElectron.setIsoR03_PuChargedHadronIso( patElectron->puChargedHadronIso() );
    if(patElectron->photonIso() != -1) localElectron.setIsoR03_PhotonIso(patElectron->photonIso());
    if(patElectron->neutralHadronIso() != -1) localElectron.setIsoR03_NeutralHadronIso(patElectron->neutralHadronIso());

    //this is for cone size r=0.4
    //following EGM POG isolation definition
    AbsVetos veto_ch;
    AbsVetos veto_nh;
    AbsVetos veto_ph;
    Direction Dir = Direction(patElectron->superCluster()->eta(), patElectron->superCluster()->phi());
    if( fabs( patElectron->superCluster()->eta() ) > 1.479 ){
      veto_ch.push_back(new reco::isodeposit::ConeVeto( Dir, 0.015 ));
      veto_ph.push_back(new reco::isodeposit::ConeVeto( Dir, 0.08 ));
    }

    //turning off access to remade isodeposits as it's not working for miniAOD
    //    const double chIso04 = patElectron->isoDeposit(pat::PfChargedHadronIso)->depositAndCountWithin(0.4, veto_ch).first;
    //const double nhIso04 = patElectron->isoDeposit(pat::PfNeutralHadronIso)->depositAndCountWithin(0.4, veto_nh).first;
    //const double phIso04 = patElectron->isoDeposit(pat::PfGammaIso)->depositAndCountWithin(0.4, veto_ph).first;
    //const double puChIso04 = patElectron->isoDeposit(pat::PfPUChargedHadronIso)->depositAndCountWithin(0.4, veto_ch).first;

    //localElectron.setIsoR04_ChargedHadronIso( chIso04 );
    //localElectron.setIsoR04_PuChargedHadronIso( puChIso04 );
    //localElectron.setIsoR04_PhotonIso( phIso04 );
    //localElectron.setIsoR04_NeutralHadronIso( nhIso04 );

    localElectron.setPassConversion(patElectron->passConversionVeto());
    localElectron.setSigmaIphiIphi( patElectron->sigmaIphiIphi() );
    localElectron.setSigmaIetaIphi( patElectron->sigmaIetaIphi() );
    localElectron.setR9( patElectron->r9() );

    // verbose printout of electron IDs available:
    if(verbosity_>2){
      std::vector<std::pair<std::string, float> > eleIds = patElectron->electronIDs();
      std::cout << "electron ID summary: " << std::endl;
      for(UInt_t ii=0; ii<eleIds.size(); ii++){
	std::cout << eleIds[ii].first << " " << eleIds[ii].second << std::endl;
      }
    }
    localElectron.setMvaTrigId( patElectron->electronID("eidRobustLoose") );
    localElectron.setMvaNonTrigId( patElectron->electronID("eidRobustTight") );

    new( (*rootElectrons)[j] ) TRootElectron(localElectron);
    if(verbosity_>2) cout << "   ["<< setw(3) << j << "] " << localElectron << endl;
    //    cout <<"size of array "<< sizeof(rootElectrons)/sizeof(*rootElectrons)  <<  " j  =  "<< j << endl;

}





}
