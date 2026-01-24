// =============== AXI4-Lite Interface ==================

interface axi4_aw_if ();
  wire AWVALID;
  wire AWREADY;
  wire [31:0] AWADDR;

  modport master (
    output AWVALID,
    input  AWREADY,
    output AWADDR
  );
endinterface

interface axi4_w_if ();
  wire WVALID;
  wire WREADY;
  wire [31:0] WDATA;
  wire [3:0] WSTRB;
endinterface

interface axi4_b_if ();
  wire BVALID;
  wire BREADY;
  wire [1:0] BRESP;
endinterface

interface axi4_ar_if ();
  wire ARVALID;
  wire ARREADY;
  wire [31:0] ARADDR;
//	wire [3:0] ARPROT;
endinterface

interface axi4_r_if ();
  wire RVALID;
  wire RREADY;
  wire [31:0] RDATA;
  wire [1:0] RRESP;
endinterface

interface axi4_lite_if ();
  wire ACLK;
  wire ARESETn;

  axi4_aw_if aw();
  axi4_w_if w();
  axi4_b_if b();
  axi4_ar_if ar();
  axi4_r_if r();


  // ================= Slave connect to ground =================
  task automatic slave_form_GND();
    aw.AWREADY = 1'b0;
    w.WREADY   = 1'b0;
    b.BVALID   = 1'b0;
    b.BRESP    = 2'b00;
    ar.ARREADY = 1'b0;
    r.RVALID   = 1'b0;
    r.RDATA    = '0;
    r.RRESP    = 2'b00;
  endtask

  // ================= Master connect to ground =================
  task automatic master_form_GND();
    aw.AWVALID = 1'b0;
    aw.AWADDR  = '0;
    w.WVALID   = 1'b0;
    w.WDATA    = '0;
    w.WSTRB    = '0;
    b.BREADY   = 1'b0;
    ar.ARVALID = 1'b0;
    ar.ARADDR  = '0;
    r.RREADY   = 1'b0;
  endtask


endinterface // axi4_lite_if


package axi4_lite_pkg;

	// =============== AXI4-Lite Master registers ==================

	typedef struct packed {
    reg        AWVALID;
    reg [31:0] AWADDR;
  } axi4_aw_Mreg;

  typedef struct packed {
    reg        WVALID;
    reg [31:0] WDATA;
    reg [3:0]  WSTRB;
  } axi4_w_Mreg;

  typedef struct packed {
    reg BREADY;
  } axi4_b_Mreg;

  typedef struct packed {
    reg        ARVALID;
    reg [31:0] ARADDR;
  } axi4_ar_Mreg;

  typedef struct packed {
    reg RREADY;
  } axi4_r_Mreg;

  typedef struct packed {
    axi4_aw_Mreg aw;
    axi4_w_Mreg  w;
    axi4_b_Mreg  b;
    axi4_ar_Mreg ar;
    axi4_r_Mreg  r;
  } axi4_lite_Mreg;

	// =============== AXI4-Lite Slave registers ==================

  typedef struct packed {
    reg AWREADY;
  } axi4_aw_Sreg;

  typedef struct packed {
    reg WREADY;
  } axi4_w_Sreg;

  typedef struct packed {
    reg        BVALID;
    reg [1:0]  BRESP;
  } axi4_b_Sreg;

  typedef struct packed {
    reg ARREADY;
  } axi4_ar_Sreg;

  typedef struct packed {
    reg        RVALID;
    reg [31:0] RDATA;
    reg [1:0]  RRESP;
  } axi4_r_Sreg;

  typedef struct packed {
    axi4_aw_Sreg aw;
    axi4_w_Sreg  w;
    axi4_b_Sreg  b;
    axi4_ar_Sreg ar;
    axi4_r_Sreg  r;
  } axi4_lite_Sreg;

endpackage

