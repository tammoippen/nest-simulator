/*
 *  iaf_chs_2007.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef IAF_CHS_2007_H
#define IAF_CHS_2007_H

#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "universal_data_logger.h"
#include "recordables_map.h"
#include "normal_randomdev.h"

namespace nest
{
  class Network;

  /* BeginDocumentation
     Name: iaf_chs_2007 - Spike-response model used in Carandini et al 2007.

     Description:
     The membrane potential is the sum of stereotyped events: the postsynaptic
     potentials (V_syn), waveforms that include a spike and the subsequent
     after-hyperpolarization (V_spike) and Gaussian-distributed white noise.

     The postsynaptic potential is described by alpha function where where
     U_epsp is the maximal amplitude of the EPSP and tau_epsp is the time to
     peak of the EPSP.

     The spike waveform is described as a delta peak followed by a membrane
     potential reset and exponential decay. U_reset is the magnitude of the
     reset/after-hyperpolarization and tau_reset is the time constant of
     recovery from this hyperpolarization.

     The linear subthresold dynamics is integrated by the Exact
     Integration scheme [1]. The neuron dynamics is solved on the time
     grid given by the computation step size. Incoming as well as emitted
     spikes are forced to that grid.

     Note:
     The way the noise term was implemented in the original model makes it
     unsuitable for simulation in NEST. The workaround was to prepare the
     noise signal externally prior to simulation. The noise signal,
     if present, has to be at least as long as the simulation.

     Parameters: 
     The following parameters can be set in the status dictionary.

     tau_epsp       double - Membrane time constant in ms.
     tau_reset      double - Refractory time constant in ms.
     U_epsp         double - Maximum amplitude of the EPSP. Normalized.
     U_reset        double - Reset value of the membrane potential. Normalized.
     U_noise        double - Noise scale. Normalized.
     noise   vector<double>- Noise signal.
 
     References:
     [1] Carandini M, Horton JC, Sincich LC (2007) Thalamic filtering of retinal
     spike trains by postsynaptic summation. J Vis 7(14):20,1�11.
     [2] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
     systems with applications to neuronal modeling. Biologial Cybernetics
     81:381-402.

     Sends: SpikeEvent

     Receives: SpikeEvent, DataLoggingRequest

     FirstVersion: May 2012
     Author: Thomas Heiberg, Birgit Kriener
  */

  /**
   * Neuron model used in Carandini et al 2007.
   */
  class iaf_chs_2007:
  public Archiving_Node 
  {
    
  public:        
    
    iaf_chs_2007();
    iaf_chs_2007(const iaf_chs_2007&);

    /**
     * Import sets of overloaded virtual functions.
     * We need to explicitly include sets of overloaded
     * virtual functions into the current scope.
     * According to the SUN C++ FAQ, this is the correct
     * way of doing things, although all other compilers
     * happily live without.
     */

    using Node::connect_sender;
    using Node::handle;

    port check_connection(Connection&, port);
    
    void handle(SpikeEvent &);
    void handle(DataLoggingRequest &);
    
    port connect_sender(SpikeEvent &, port);
    port connect_sender(DataLoggingRequest &, port);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);

  private:

    void init_node_(const Node& proto);
    void init_state_(const Node& proto);
    void init_buffers_();
    void calibrate();

    void update(const Time &, const long_t, const long_t);

    // The next two classes need to be friends to access the State_ class/member
    friend class RecordablesMap<iaf_chs_2007>;
    friend class UniversalDataLogger<iaf_chs_2007>;

    // ----------------------------------------------------------------

    /**
     * State variables of the model.
     */
    struct State_
    {
      // state variables
      double_t i_syn_ex_;  // postsynaptic current for exc. inputs, variable 1
      double_t V_syn_;	   // psp waveform, variable 2
      double_t V_spike_;	 // post spike reset waveform, variable 3
      double_t V_m_;       // membrane potential, variable 4

      ulong_t position_;

      State_();  //!< Default initialization

      void get(DictionaryDatum &) const;
      void set(DictionaryDatum const &);
    };

    // ----------------------------------------------------------------

    /** 
     * Independent parameters of the model. 
     */
    struct Parameters_
    {
  
      /** Membrane time constant in ms. */
      double_t tau_epsp_;

      /** Refractory time constant in ms. */
      double_t tau_reset_;

      /** Resting potential. Normalized = 0.0. */
      double_t E_L_;

      /** Threshold. Normalized = 1.0. */
      double_t U_th_;

      /** Normalized maximum amplitude of the EPSP. */
      double_t U_epsp_;

      /** Normalized magnitude of the membrane potential reset. */
      double_t U_reset_;

      /** Membrane capacitance. Note: Does not have any function currently. */
      double_t C_;

      /** Noise scale. */
      double_t U_noise_;

      /** Noise signal. */
      std::vector<double_t> noise_;

      Parameters_();  //!< Sets default parameter values

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary

      /** Set values from dictionary.
       * @returns Change in reversal potential E_L, to be passed to State_::set()
       * @note State is passed so that the position can be reset if the
       *       noise_ vector has been filled with new data.
       */
      void set(const DictionaryDatum&, State_& s);
    };
    

    // ----------------------------------------------------------------

    /**
     * Buffers of the model.
     */
    struct Buffers_
    {
      Buffers_(iaf_chs_2007 &);
      Buffers_(const Buffers_ &, iaf_chs_2007 &);

      /** buffers and sums up incoming spikes/currents */
      RingBuffer spikes_ex_;
      RingBuffer currents_;  

      //! Logger for all analog data
      UniversalDataLogger<iaf_chs_2007> logger_;
    };
    
    // ---------------------------------------------------------------- 

    /**
     * Internal variables of the model.
     */
    struct Variables_
    { 
      /** Amplitude of the synaptic current.
	  This value is chosen such that a post-synaptic potential with
	  weight one has an amplitude of 1 mV.
	  @note mog - I assume this, not checked. 
      */
      //    double_t PSCInitialValue_;
    
      // time evolution operator
      double_t P20_;
      double_t P11ex_;
      double_t P21ex_;
      double_t P22_;
      double_t P30_;

      librandom::NormalRandomDev normal_dev_;  //!< random deviate generator
    };

    // Access functions for UniversalDataLogger -------------------------------

    //! Read out the real membrane potential
    double_t get_V_m_() const { return S_.V_m_ + P_.E_L_; }

    // ---------------------------------------------------------------- 

    /**
     * @defgroup iaf_psc_exp_data
     * Instances of private data structures for the different types
     * of data pertaining to the model.
     * @note The order of definitions is important for speed.
     * @{
     */   
    Parameters_ P_;
    State_      S_;
    Variables_  V_;
    Buffers_    B_;
    /** @} */

    //! Mapping of recordables names to access functions
    static RecordablesMap<iaf_chs_2007> recordablesMap_;
  };

  inline
  port iaf_chs_2007::check_connection(Connection &c, port receptor_type)
  {
    SpikeEvent e;
    e.set_sender(*this);
    c.check_event(e);
    return c.get_target()->connect_sender(e, receptor_type);
  }

  inline
  port iaf_chs_2007::connect_sender(SpikeEvent&, port receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return 0;
  }
 
  inline
  port iaf_chs_2007::connect_sender(DataLoggingRequest &dlr,
				   port receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return B_.logger_.connect_logging_device(dlr, recordablesMap_); 
  }

  inline
  void iaf_chs_2007::get_status(DictionaryDatum &d) const
  {
    P_.get(d);
    S_.get(d);
    Archiving_Node::get_status(d);

    (*d)[names::recordables] = recordablesMap_.get_list();
  }

  inline
  void iaf_chs_2007::set_status(const DictionaryDatum &d)
  {
    Parameters_ ptmp = P_;  // temporary copy in case of errors
    ptmp.set(d, S_);
    State_      stmp = S_;  // temporary copy in case of errors
    stmp.set(d);                 // throws if BadProperty

    // We now know that (ptmp, stmp) are consistent. We do not 
    // write them back to (P_, S_) before we are also sure that 
    // the properties to be set in the parent class are internally 
    // consistent.
    Archiving_Node::set_status(d);

    // if we get here, temporaries contain consistent set of properties
    P_ = ptmp;
    S_ = stmp;
  }

} // namespace

#endif // IAF_CHS_2007_H
