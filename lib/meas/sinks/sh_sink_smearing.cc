// $Id: sh_sink_smearing.cc,v 3.4 2006-06-10 16:28:52 edwards Exp $
/*! \file
 *  \brief Shell sink smearing
 */

#include "chromabase.h"
#include "handle.h"

#include "meas/sinks/sink_smearing_factory.h"
#include "meas/sinks/sh_sink_smearing.h"

#include "meas/smear/link_smearing_factory.h"
#include "meas/smear/link_smearing_aggregate.h"

#include "meas/smear/quark_smearing_factory.h"
#include "meas/smear/quark_smearing_aggregate.h"

#include "meas/smear/quark_smearing.h"

#include "meas/smear/quark_displacement_aggregate.h"
#include "meas/smear/quark_displacement_factory.h"

#include "meas/smear/simple_quark_displacement.h"
#include "meas/smear/no_quark_displacement.h"

namespace Chroma
{
  // Read parameters
  void read(XMLReader& xml, const string& path, ShellQuarkSinkSmearingEnv::Params& param)
  {
    ShellQuarkSinkSmearingEnv::Params tmp(xml, path);
    param = tmp;
  }

  // Writer
  void write(XMLWriter& xml, const string& path, const ShellQuarkSinkSmearingEnv::Params& param)
  {
    param.writeXML(xml, path);
  }



  //! Hooks to register the class
  namespace ShellQuarkSinkSmearingEnv
  {
    //! Callback function
    QuarkSourceSink<LatticePropagator>* createProp(XMLReader& xml_in,
						   const std::string& path,
						   const multi1d<LatticeColorMatrix>& u)
    {
      return new SinkSmear<LatticePropagator>(Params(xml_in, path), u);
    }

    //! Callback function
    QuarkSourceSink<LatticeFermion>* createFerm(XMLReader& xml_in,
						const std::string& path,
						const multi1d<LatticeColorMatrix>& u)
    {
      return new SinkSmear<LatticeFermion>(Params(xml_in, path), u);
    }

    //! Name to be used
    const std::string name("SHELL_SINK");

    //! Register all the factories
    bool registerAll()
    {
      bool foo = true;
      foo &= LinkSmearingEnv::registered;
      foo &= QuarkSmearingEnv::registered;
      foo &= QuarkDisplacementEnv::registered;
      foo &= Chroma::ThePropSinkSmearingFactory::Instance().registerObject(name, createProp);
      foo &= Chroma::TheFermSinkSmearingFactory::Instance().registerObject(name, createFerm);
      return foo;
    }

    //! Register the sink smearing
    const bool registered = registerAll();


    //! Initialize
    Params::Params()
    {
      quark_smear_firstP = true;
    }


    //! Read parameters
    Params::Params(XMLReader& xml, const string& path)
    {
      XMLReader paramtop(xml, path);

      int version;
      read(paramtop, "version", version);

      quark_smear_firstP = true;

      switch (version) 
      {
      case 1:
      {
	quark_displacement.id = SimpleQuarkDisplacementEnv::name;
	int disp_length = 0;
	int disp_dir = 0;

	XMLBufferWriter xml_tmp;
	push(xml_tmp, "Displacement");
	write(xml_tmp, "DisplacementType", quark_displacement.id);

	if (paramtop.count("disp_length") != 0)
	  read(paramtop, "disp_length", disp_length);

	if (paramtop.count("disp_dir") != 0)
	  read(paramtop, "disp_dir", disp_dir);

	write(xml_tmp, "disp_length", disp_length);
	write(xml_tmp, "disp_dir",  disp_dir);

	pop(xml_tmp);  // Displacement

	quark_displacement.xml = xml_tmp.printCurrentContext();
      }
      break;

      case 2:
      {
	if (paramtop.count("Displacement") != 0)
	  quark_displacement = readXMLGroup(paramtop, "Displacement", "DisplacementType");
	else
	  quark_displacement = QuarkDisplacementEnv::nullXMLGroup();
      }
      break;

      case 3:
      {
	read(paramtop, "quark_smear_firstP", quark_smear_firstP);

	if (paramtop.count("Displacement") != 0)
	  quark_displacement = readXMLGroup(paramtop, "Displacement", "DisplacementType");
	else
	  quark_displacement = QuarkDisplacementEnv::nullXMLGroup();
      }
      break;

      default:
	QDPIO::cerr << __func__ << ": parameter version " << version 
		    << " unsupported." << endl;
	QDP_abort(1);
      }

      quark_smearing = readXMLGroup(paramtop, "SmearingParam", "wvf_kind");

      if (paramtop.count("LinkSmearing") != 0)
	link_smearing = readXMLGroup(paramtop, "LinkSmearing", "LinkSmearingType");
      else
	link_smearing = LinkSmearingEnv::nullXMLGroup();
    }


    // Writer
    void Params::writeXML(XMLWriter& xml, const string& path) const
    {
      push(xml, path);

      int version = 3;
      write(xml, "version", version);

      write(xml, "SinkType", ShellQuarkSinkSmearingEnv::name);
      xml << quark_smearing.xml;
      xml << quark_displacement.xml;
      xml << link_smearing.xml;
      write(xml, "quark_smear_firstP", quark_smear_firstP);
      pop(xml);

      pop(xml);
    }



    //! Smear the sink
    template<>
    void
    SinkSmear<LatticePropagator>::operator()(LatticePropagator& quark_sink) const
    {
      QDPIO::cout << "Shell sink" << endl;

      try
      {
	//
	// Create the quark smearing object
	//
	std::istringstream  xml_s(params.quark_smearing.xml);
	XMLReader  smeartop(xml_s);
	const string smear_path = "/SmearingParam";
        QDPIO::cout << "Quark smearing type = " << params.quark_smearing.id << endl;
	
	Handle< QuarkSmearing<LatticePropagator> >
	  quarkSmearing(ThePropSmearingFactory::Instance().createObject(params.quark_smearing.id,
									smeartop,
									smear_path));

	//
	// Create the quark displacement object
	//
	std::istringstream  xml_d(params.quark_displacement.xml);
	XMLReader  displacetop(xml_d);
	const string displace_path = "/Displacement";
        QDPIO::cout << "Displacement type = " << params.quark_displacement.id << endl;
	
	Handle< QuarkDisplacement<LatticePropagator> >
	  quarkDisplacement(ThePropDisplacementFactory::Instance().createObject(params.quark_displacement.id,
										displacetop,
										displace_path));

	if (params.quark_smear_firstP)
	{
	  //
	  // Sink smear quark
	  //
	  (*quarkSmearing)(quark_sink, u_smr);

	  //
	  // Displace the sink
	  //
	  (*quarkDisplacement)(quark_sink, u_smr, PLUS);
	}
	else
	{
	  //
	  // Displace the sink
	  //
	  (*quarkDisplacement)(quark_sink, u_smr, PLUS);

	  //
	  // Sink smear quark
	  //
	  (*quarkSmearing)(quark_sink, u_smr);
	}

      }
      catch(const std::string& e) 
      {
	QDPIO::cerr << name << ": Caught Exception smearing: " << e << endl;
	QDP_abort(1);
      }
    }



    //! Smear the sink
    template<>
    void
    SinkSmear<LatticeFermion>::operator()(LatticeFermion& quark_sink) const
    {
//      QDPIO::cout << "Shell sink" << endl;

      try
      {
	//
	// Create the quark smearing object
	//
	std::istringstream  xml_s(params.quark_smearing.xml);
	XMLReader  smeartop(xml_s);
	const string smear_path = "/SmearingParam";
	
	Handle< QuarkSmearing<LatticeFermion> >
	  quarkSmearing(TheFermSmearingFactory::Instance().createObject(params.quark_smearing.id,
									smeartop,
									smear_path));

	//
	// Create the quark displacement object
	//
	std::istringstream  xml_d(params.quark_displacement.xml);
	XMLReader  displacetop(xml_d);
	const string displace_path = "/Displacement";
	
	Handle< QuarkDisplacement<LatticeFermion> >
	  quarkDisplacement(TheFermDisplacementFactory::Instance().createObject(params.quark_displacement.id,
										displacetop,
										displace_path));

	//
	// Sink smear quark
	//
	(*quarkSmearing)(quark_sink, u_smr);


	//
	// Displace the sink
	//
	(*quarkDisplacement)(quark_sink, u_smr, PLUS);

      }
      catch(const std::string& e) 
      {
	QDPIO::cerr << name << ": Caught Exception smearing: " << e << endl;
	QDP_abort(1);
      }
    }

  }

}
