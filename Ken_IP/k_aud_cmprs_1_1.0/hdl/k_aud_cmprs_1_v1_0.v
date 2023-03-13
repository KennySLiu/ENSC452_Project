
`timescale 1 ns / 1 ps

    module k_aud_cmprs_1_v1_0 #
    (
        // Users to add parameters here

        // Number of FFT points.
        parameter integer K_FFT_NUM_PTS         = 16,

        // User parameters ends
        // Do not modify the parameters beyond this line


        // Parameters of Axi Slave Bus Interface S_FFT_IN
        parameter integer C_S_FFT_IN_TDATA_WIDTH    = 32,

        // Parameters of Axi Slave Bus Interface S_SQRT_IN
        parameter integer C_S_SQRT_IN_TDATA_WIDTH    = 32,

        // Parameters of Axi Master Bus Interface M_FFT_OUT
        parameter integer C_M_FFT_OUT_TDATA_WIDTH    = 32,
        parameter integer C_M_FFT_OUT_START_COUNT    = 32,

        // Parameters of Axi Master Bus Interface M_SQRT_OUT
        parameter integer C_M_SQRT_OUT_TDATA_WIDTH    = 32,
        parameter integer C_M_SQRT_OUT_START_COUNT    = 32
    )
    (
        // Users to add ports here

        // User ports ends
        // Do not modify the ports beyond this line


        // Ports of Axi Slave Bus Interface S_FFT_IN
        input wire  s_fft_in_aclk,
        input wire  s_fft_in_aresetn,
        output wire  s_fft_in_tready,
        input wire [C_S_FFT_IN_TDATA_WIDTH-1 : 0] s_fft_in_tdata,
        input wire [(C_S_FFT_IN_TDATA_WIDTH/8)-1 : 0] s_fft_in_tstrb,
        input wire  s_fft_in_tlast,
        input wire  s_fft_in_tvalid,

        // Ports of Axi Slave Bus Interface S_SQRT_IN
        input wire  s_sqrt_in_aclk,
        input wire  s_sqrt_in_aresetn,
        output wire  s_sqrt_in_tready,
        input wire [C_S_SQRT_IN_TDATA_WIDTH-1 : 0] s_sqrt_in_tdata,
        input wire [(C_S_SQRT_IN_TDATA_WIDTH/8)-1 : 0] s_sqrt_in_tstrb,
        input wire  s_sqrt_in_tlast,
        input wire  s_sqrt_in_tvalid,

        // Ports of Axi Master Bus Interface M_FFT_OUT
        input wire  m_fft_out_aclk,
        input wire  m_fft_out_aresetn,
        output wire  m_fft_out_tvalid,
        output wire [C_M_FFT_OUT_TDATA_WIDTH-1 : 0] m_fft_out_tdata,
        output wire [(C_M_FFT_OUT_TDATA_WIDTH/8)-1 : 0] m_fft_out_tstrb,
        output wire  m_fft_out_tlast,
        input wire  m_fft_out_tready,

        // Ports of Axi Master Bus Interface M_SQRT_OUT
        input wire  m_sqrt_out_aclk,
        input wire  m_sqrt_out_aresetn,
        output wire  m_sqrt_out_tvalid,
        output wire [C_M_SQRT_OUT_TDATA_WIDTH-1 : 0] m_sqrt_out_tdata,
        output wire [(C_M_SQRT_OUT_TDATA_WIDTH/8)-1 : 0] m_sqrt_out_tstrb,
        output wire  m_sqrt_out_tlast,
        input wire  m_sqrt_out_tready
    );

    //////////////////////
    //////////////////////
    // KENNYS CODE
    localparam integer DATA_WIDTH = C_S_FFT_IN_TDATA_WIDTH/2;

    // Define the states of state machine
    parameter [3:0] IDLE = 4'b0000,
                    READ_FFT_DATA_AND_COMPUTE_ENERGY    = 4'b0001,
                    COMPUTE_INITIAL_MULTIPLIER          = 4'b0010,
                    COMPUTE_SQRT_MULTIPLIER             = 4'b0011,
                    SEND_STREAM     = 4'b0100;
    reg [3:0] k_cur_state;
    reg [3:0] k_prev_state;
    reg [31:0] fftin_count;

    reg [DATA_WIDTH - 1 : 0] fft_in_re;
    reg [DATA_WIDTH - 1 : 0] fft_in_im;

    always @(posedge m_fft_out_aclk)
    begin
        k_prev_state <= k_cur_state;

        if (!m_fft_out_aresetn) begin
        // Synchronous reset (active low)
                k_cur_state <= IDLE;
                cur_st_done <= 1;
            end
        else begin
            case (k_cur_state)
                IDLE:
                    k_cur_state <= READ_FFT_DATA_AND_COMPUTE_ENERGY;

                READ_FFT_DATA_AND_COMPUTE_ENERGY:
                    if (s_fft_in_tlast == 1'b1) begin
                        k_cur_state <= COMPUTE_INITIAL_MULTIPLIER;
                    end
                    else begin
                        k_cur_state <= READ_FFT_DATA_AND_COMPUTE_ENERGY;
                    end

                COMPUTE_INITIAL_MULTIPLIER:

                COMPUTE_SQRT_MULTIPLIER:

                APPLY_MULTIPLIER: 

                SEND_STREAM:


            endcase
        end
    end

    // Reading in the FFT data
    always @(posedge m_fft_out_aclk)
    begin
        if (s_fft_in_tready & s_fft_in_tvalid)
        begin
            fft_in_re[DATA_WIDTH - 1: 0] <= s_fft_in_tdata[DATA_WIDTH - 1: 0];
            fft_in_im[DATA_WIDTH - 1: 0] <= s_fft_in_tdata[C_S_FFT_IN_TDATA_WIDTH : DATA_WIDTH];
        end
    end

    always @(posedge m_fft_out_aclk)
    begin
        s_fft_in_tready <= (k_cur_state == READ_FFT_DATA_AND_COMPUTE_ENERGY) &&
                           ();

        total_energy += fft_in_re * fft_in_re;
        total_energy += fft_in_im * fft_in_im;

    end


    // END OF KENNYS CODE
    //////////////////////
    //////////////////////

//// Instantiation of Axi Bus Interface S_FFT_IN
//    k_aud_cmprs_1_v1_0_S_FFT_IN # (
//        .NUMBER_OF_INPUT_WORDS(K_FFT_NUM_PTS);
//        .C_S_AXIS_TDATA_WIDTH(C_S_FFT_IN_TDATA_WIDTH)
//    ) k_aud_cmprs_1_v1_0_S_FFT_IN_inst (
//        .S_AXIS_ACLK(s_fft_in_aclk),
//        .S_AXIS_ARESETN(s_fft_in_aresetn),
//        .S_AXIS_TREADY(s_fft_in_tready),
//        .S_AXIS_TDATA(s_fft_in_tdata),
//        .S_AXIS_TSTRB(s_fft_in_tstrb),
//        .S_AXIS_TLAST(s_fft_in_tlast),
//        .S_AXIS_TVALID(s_fft_in_tvalid)
//    );
//
//// Instantiation of Axi Bus Interface S_SQRT_IN
//    k_aud_cmprs_1_v1_0_S_SQRT_IN # (
//        .C_S_AXIS_TDATA_WIDTH(C_S_SQRT_IN_TDATA_WIDTH)
//    ) k_aud_cmprs_1_v1_0_S_SQRT_IN_inst (
//        .S_AXIS_ACLK(s_sqrt_in_aclk),
//        .S_AXIS_ARESETN(s_sqrt_in_aresetn),
//        .S_AXIS_TREADY(s_sqrt_in_tready),
//        .S_AXIS_TDATA(s_sqrt_in_tdata),
//        .S_AXIS_TSTRB(s_sqrt_in_tstrb),
//        .S_AXIS_TLAST(s_sqrt_in_tlast),
//        .S_AXIS_TVALID(s_sqrt_in_tvalid)
//    );
//
//// Instantiation of Axi Bus Interface M_FFT_OUT
//    k_aud_cmprs_1_v1_0_M_FFT_OUT # (
//        .NUMBER_OF_OUTPUT_WORDS(K_FFT_NUM_PTS);
//        .C_M_AXIS_TDATA_WIDTH(C_M_FFT_OUT_TDATA_WIDTH),
//        .C_M_START_COUNT(C_M_FFT_OUT_START_COUNT)
//    ) k_aud_cmprs_1_v1_0_M_FFT_OUT_inst (
//        .M_AXIS_ACLK(m_fft_out_aclk),
//        .M_AXIS_ARESETN(m_fft_out_aresetn),
//        .M_AXIS_TVALID(m_fft_out_tvalid),
//        .M_AXIS_TDATA(m_fft_out_tdata),
//        .M_AXIS_TSTRB(m_fft_out_tstrb),
//        .M_AXIS_TLAST(m_fft_out_tlast),
//        .M_AXIS_TREADY(m_fft_out_tready)
//    );
//
//// Instantiation of Axi Bus Interface M_SQRT_OUT
//    k_aud_cmprs_1_v1_0_M_SQRT_OUT # (
//        .C_M_AXIS_TDATA_WIDTH(C_M_SQRT_OUT_TDATA_WIDTH),
//        .C_M_START_COUNT(C_M_SQRT_OUT_START_COUNT)
//    ) k_aud_cmprs_1_v1_0_M_SQRT_OUT_inst (
//        .M_AXIS_ACLK(m_sqrt_out_aclk),
//        .M_AXIS_ARESETN(m_sqrt_out_aresetn),
//        .M_AXIS_TVALID(m_sqrt_out_tvalid),
//        .M_AXIS_TDATA(m_sqrt_out_tdata),
//        .M_AXIS_TSTRB(m_sqrt_out_tstrb),
//        .M_AXIS_TLAST(m_sqrt_out_tlast),
//        .M_AXIS_TREADY(m_sqrt_out_tready)
//    );

    // Add user logic here

    // User logic ends

    endmodule
