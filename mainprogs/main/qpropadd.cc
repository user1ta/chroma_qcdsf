// $Id: qpropadd.cc,v 1.1 2004-04-12 15:42:42 edwards Exp $
/*! \file
 * \brief Add two quark propagators
 *
 * Simple program for adding two quark props
 */

#include "chroma.h"

using namespace QDP;


/*
 * Input 
 */
//! Parameters for running program
struct Param_t
{
  multi1d<int> nrow;
};

//! Propagators
struct Prop_t
{
  string           prop_file1;  // The files is expected to be in SciDAC format!
  string           prop_file2;  // The files is expected to be in SciDAC format!

  string           prop_file;   // The files is expected to be in SciDAC format!
  QDP_volfmt_t     prop_volfmt;
};


//! Mega-structure of all input
struct Qpropadd_input_t
{
  Param_t          param;
  Prop_t           prop;
};


//! Propagator parameters
void read(XMLReader& xml, const string& path, Prop_t& input)
{
  XMLReader inputtop(xml, path);

  read(inputtop, "prop_file1", input.prop_file1);
  read(inputtop, "prop_file2", input.prop_file2);
}


// Reader for input parameters
void read(XMLReader& xml, const string& path, Param_t& param)
{
  XMLReader paramtop(xml, path);

  int version;
  read(paramtop, "version", version);

  switch (version) 
  {
  case 1:
    /**************************************************************************/
    break;

  default :
    /**************************************************************************/

    QDPIO::cerr << "Input parameter version " << version << " unsupported." << endl;
    QDP_abort(1);
  }

  read(paramtop, "nrow", param.nrow);
}


// Reader for input parameters
void read(XMLReader& xml, const string& path, Qpropadd_input_t& input)
{
  XMLReader inputtop(xml, path);

  // Read all the input groups
  try
  {
    // Read program parameters
    read(inputtop, "Param", input.param);

    // Read in the propagator(s) info
    read(inputtop, "Prop", input.prop);
  }
  catch (const string& e) 
  {
    QDPIO::cerr << "Error reading qpropadd data: " << e << endl;
    throw;
  }
}

//--------------------------------------------------------------




//! Main program for adding propagators
/*! Main program */
int
main(int argc, char *argv[])
{
  // Put the machine into a known state
  QDP_initialize(&argc, &argv);

  // Input parameter structure
  Qpropadd_input_t  input;

  // Instantiate xml reader for DATA
  XMLReader xml_in("DATA");

  // Read data
  read(xml_in, "/Qpropadd", input);

  // Specify lattice size, shape, etc.
  Layout::setLattSize(input.param.nrow);
  Layout::create();

  QDPIO::cout << " QPROPADD: Add 2 propagators" << endl;

  // Instantiate XML writer for XMLDAT
  XMLFileWriter xml_out("XMLDAT");
  push(xml_out, "Qpropadd");

  proginfo(xml_out);    // Print out basic program info

  // Write out the input
  write(xml_out, "Input", xml_in);

  push(xml_out, "Output_version");
  write(xml_out, "out_version", 1);
  pop(xml_out);

  xml_out.flush();

  //
  // Read the first quark propagator and extract headers
  //
  LatticePropagator quark_propagator1;
  ChromaProp_t prop_header1;
  PropSource_t source_header1;
  XMLReader gauge_xml1;
  {
    XMLReader prop_file_xml, prop_record_xml;
    QDPIO::cout << "Attempt to read first forward propagator" << endl;
    readQprop(prop_file_xml, 
	      prop_record_xml, quark_propagator1,
	      input.prop.prop_file1, QDPIO_SERIAL);
   
    // Try to invert this record XML into a ChromaProp struct
    // Also pull out the id of this source
    try
    {
      read(prop_record_xml, "/Propagator/ForwardProp", prop_header1);
      read(prop_record_xml, "/Propagator/PropSource", source_header1);
      read(prop_record_xml, "/Propagator/Config_info", gauge_xml1);
    }
    catch (const string& e) 
    {
      QDPIO::cerr << "Error extracting forward_prop header: " << e << endl;
      throw;
    }
  }
  QDPIO::cout << "First forward propagator successfully read" << endl;


  // Sanity check - write out the norm2 of the forward prop in the j_decay direction
  // Use this for any possible verification
  {
    // Initialize the slow Fourier transform phases
    SftMom phases(0, true, source_header1.j_decay);

    multi1d<Double> forward_prop_corr = sumMulti(localNorm2(quark_propagator1), 
						 phases.getSet());

    push(xml_out, "Forward_prop_correlator1");
    write(xml_out, "forward_prop_corr", forward_prop_corr);
    pop(xml_out);
  }

  // Save prop input
  write(xml_out, "ForwardProp1", prop_header1);
  write(xml_out, "PropSource1", source_header1);


  //
  // Read the second quark propagator and extract headers
  //
  LatticePropagator quark_propagator2;
  ChromaProp_t prop_header2;
  PropSource_t source_header2;
  {
    XMLReader prop_file_xml, prop_record_xml;
    QDPIO::cout << "Attempt to read first forward propagator" << endl;
    readQprop(prop_file_xml, 
	      prop_record_xml, quark_propagator2,
	      input.prop.prop_file2, QDPIO_SERIAL);
   
    // Try to invert this record XML into a ChromaProp struct
    // Also pull out the id of this source
    try
    {
      read(prop_record_xml, "/Propagator/ForwardProp", prop_header2);
      read(prop_record_xml, "/Propagator/PropSource", source_header2);
    }
    catch (const string& e) 
    {
      QDPIO::cerr << "Error extracting forward_prop header: " << e << endl;
      throw;
    }
  }
  QDPIO::cout << "Second forward propagator successfully read" << endl;

  // Sanity check - write out the norm2 of the forward prop in the j_decay direction
  // Use this for any possible verification
  {
    // Initialize the slow Fourier transform phases
    SftMom phases(0, true, source_header2.j_decay);

    multi1d<Double> forward_prop_corr = sumMulti(localNorm2(quark_propagator2), 
						 phases.getSet());

    push(xml_out, "Forward_prop_correlator1");
    write(xml_out, "forward_prop_corr", forward_prop_corr);
    pop(xml_out);
  }

  // Save prop input
  write(xml_out, "ForwardProp2", prop_header2);
  write(xml_out, "PropSource2", source_header2);



  // More sanity checks
  if (source_header1.j_decay != source_header2.j_decay)
  {
    QDPIO::cerr << "Prop j_decay mismatch" << endl;
    QDP_abort(1);
  }

  for(int i=0; i < source_header1.t_source.size(); ++i)
    if (source_header1.t_source[i] != source_header2.t_source[i])
    {
      QDPIO::cerr << "Prop t_source mismatch" << endl;
      QDP_abort(1);
    }

  for(int i=0; i < prop_header1.boundary.size(); ++i)
  {
    if ((i == source_header1.j_decay) && 
	(prop_header1.boundary[i] != -prop_header2.boundary[i]))
    {
      QDPIO::cerr << "Expect two props to have opposite BC" << endl;
      QDP_abort(1);
    }

    if ((i != source_header1.j_decay) && 
	(prop_header1.boundary[i] != prop_header2.boundary[i]))
    {
      QDPIO::cerr << "Prop boundary mismatch" << endl;
      QDP_abort(1);
    }
  }


  /*
   * Add the props and be done...
   */
  LatticePropagator  quark_propagator = 0.5*(quark_propagator1 + quark_propagator2);
  ChromaProp_t prop_header = qprop_header1;
  PropSource_t source_header = source_header1;
  prop_header.boundary[j_decay] = 0;


  // Save the propagator
  // ONLY SciDAC output format is supported!
  {
    XMLBufferWriter file_xml;
    push(file_xml, "propagator");
    int id = 0;    // NEED TO FIX THIS - SOMETHING NON-TRIVIAL NEEDED
    write(file_xml, "id", id);
    pop(file_xml);

    XMLBufferWriter record_xml;
    push(record_xml, "Propagator");
    write(record_xml, "ForwardProp", prop_header);
    write(record_xml, "PropSource", source_header);
    write(record_xml, "Config_info", gauge_xml1);
    pop(record_xml);

    // Write the source
    writeQprop(file_xml, record_xml, quark_propagator,
	       input.prop.prop_file, input.prop.prop_volfmt, 
	       QDPIO_SERIAL);
  }


  // Close the namelist output file XMLDAT
  pop(xml_out);     // Qpropadd

  xml_in.close();
  xml_out.close();

  // Time to bolt
  QDP_finalize();

  exit(0);
}
