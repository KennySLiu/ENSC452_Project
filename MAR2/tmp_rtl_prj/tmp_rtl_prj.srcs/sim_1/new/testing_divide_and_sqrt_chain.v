`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/13/2023 01:23:29 PM
// Design Name: 
// Module Name: testing_divide_and_sqrt_chain
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module testing_divide_and_sqrt_chain(  );
    localparam IN_DWIDTH = 32;
    localparam OUT_DWIDTH = 16;
    localparam FLOAT_WIDTH = 32;

    reg                           aclk;
    //clock handle
    initial begin
        aclk = 0;
        forever #2 aclk = ~aclk;
    end
    
    
    /////////////

    // Inputs/Outputs
    reg [IN_DWIDTH-1 : 0]           ina_data;
    wire                            ina_ready;
    reg                             ina_valid;

    reg [IN_DWIDTH-1 : 0]           inb_data;
    wire                            inb_ready;
    reg                             inb_valid;

    reg [OUT_DWIDTH-1 : 0]         ACTUAL_out_data;

    wire [OUT_DWIDTH-1 : 0]         out_data_WIRE;
    reg                             out_ready;
    wire                            out_valid;

    // Connections between modules
    wire [FLOAT_WIDTH-1 : 0]        conv_a_to_div_data;
    wire                            conv_a_to_div_ready;
    wire                            conv_a_to_div_valid;
    wire [FLOAT_WIDTH-1 : 0]        conv_b_to_div_data;
    wire                            conv_b_to_div_ready;
    wire                            conv_b_to_div_valid;
    wire [FLOAT_WIDTH-1 : 0]        div_to_sqrt_data;
    wire                            div_to_sqrt_ready;
    wire                            div_to_sqrt_valid;
    wire [FLOAT_WIDTH-1 : 0]        sqrt_to_conv_data;
    wire                            sqrt_to_conv_ready;
    wire                            sqrt_to_conv_valid;
    

    // Data
    integer ina_counter;
    integer inb_counter;
    reg   [IN_DWIDTH-1:0]        ina_array[0:9];
    reg   [IN_DWIDTH-1:0]        inb_array[0:9];

    ////////////////////////////
    // Data generation


    //passing ina data
    always @(negedge aclk) begin
        if(ina_ready && ina_valid) begin
            ina_data <= ina_array[ina_counter];
            if(ina_counter == 9) 
                ina_counter = 0;
            else
                ina_counter=ina_counter+1;
        end
    end

    //passing inb data
    always @(negedge aclk) begin
        if(inb_ready && inb_valid) begin
            inb_data <= inb_array[inb_counter];
            if(inb_counter == 9) 
                inb_counter = 0;
            else
                //inb_counter=inb_counter+1;
                inb_counter=inb_counter;
        end
    end

    //reading output
    always @(negedge aclk) begin
        if (out_ready && out_valid) begin
            ACTUAL_out_data <= out_data_WIRE;
        end
    end

    initial begin
        ina_valid <= 0;
        inb_valid <= 0;
        out_ready <= 0;
        #20
        ina_valid <= 1;
        inb_valid <= 1;
        out_ready <= 1;
    end

    initial begin
        ina_array[0] = 32'h18; 
        ina_array[1] = 32'h21; 
        ina_array[2] = 32'h2a;
        ina_array[3] = 32'h33;
        ina_array[4] = 32'h3c;
        ina_array[5] = 32'h45;
        ina_array[6] = 32'h4e;
        ina_array[7] = 32'h57;
        ina_array[8] = 32'h60;
        ina_array[9] = 32'h69;
        ina_counter = 0;

        inb_array[0] = 32'h59 ;
        inb_array[1] = 32'h77 ;
        inb_array[2] = 32'h95 ;
        inb_array[3] = 32'hb3 ;
        inb_array[4] = 32'hd1 ;
        inb_array[5] = 32'hef ;
        inb_array[6] = 32'h10d;
        inb_array[7] = 32'h12b;
        inb_array[8] = 32'h149;
        inb_array[9] = 32'h167;
        inb_counter = 0;
    end



    ////////////////
    // Module instantiations
    //  - We pass in A and B to convert fixed->float.
    //  - Then compute sqrt(A/B)
    //  - And finally convert this back to fixed point.
    fixed_to_float input_converter_a (
      .aclk            (aclk),                                  // input wire aclk
      .s_axis_a_tvalid          (ina_valid),   
      .s_axis_a_tready          (ina_ready),            // output wire s_axis_a_tready
      .s_axis_a_tdata           (ina_data),              // input wire [31 : 0] s_axis_a_tdata
      .m_axis_result_tdata     (conv_a_to_div_data ),  // output wire m_axis_result_tdata
      .m_axis_result_tready     (conv_a_to_div_ready),  // input wire m_axis_result_tready
      .m_axis_result_tvalid      (conv_a_to_div_valid)    // output wire [31 : 0] m_axis_result_tvalid
    );
    fixed_to_float input_converter_b (
      .aclk            (aclk),                                  // input wire aclk
      .s_axis_a_tdata          (inb_data ),            // input wire s_axis_a_tdata
      .s_axis_a_tready          (inb_ready),            // output wire s_axis_a_tready
      .s_axis_a_tvalid           (inb_valid),              // input wire [31 : 0] s_axis_a_tvalid
      .m_axis_result_tdata     (conv_b_to_div_data ),  // output wire m_axis_result_tdata
      .m_axis_result_tready     (conv_b_to_div_ready),  // input wire m_axis_result_tready
      .m_axis_result_tvalid      (conv_b_to_div_valid)    // output wire [31 : 0] m_axis_result_tvalid
    );
    float_divide float_divider (
      .aclk            (aclk),                                  // input wire aclk
      .s_axis_a_tdata          (conv_a_to_div_data ),            // input wire s_axis_a_tdata
      .s_axis_a_tready          (conv_a_to_div_ready),            // output wire s_axis_a_tready
      .s_axis_a_tvalid           (conv_a_to_div_valid),              // input wire [31 : 0] s_axis_a_tvalid
      .s_axis_b_tdata          (conv_b_to_div_data ),            // input wire s_axis_b_tdata
      .s_axis_b_tready          (conv_b_to_div_ready),            // output wire s_axis_b_tready
      .s_axis_b_tvalid           (conv_b_to_div_valid),              // input wire [31 : 0] s_axis_b_tvalid
      .m_axis_result_tdata     (div_to_sqrt_data ),  // output wire m_axis_result_tdata
      .m_axis_result_tready     (div_to_sqrt_ready),  // input wire m_axis_result_tready
      .m_axis_result_tvalid      (div_to_sqrt_valid)    // output wire [31 : 0] m_axis_result_tvalid
    );
    float_sqrt float_sqrter (
      .aclk            (aclk),                                  // input wire aclk
      .s_axis_a_tdata          (div_to_sqrt_data ),            // input wire s_axis_a_tdata
      .s_axis_a_tready          (div_to_sqrt_ready),            // output wire s_axis_a_tready
      .s_axis_a_tvalid           (div_to_sqrt_valid),              // input wire [31 : 0] s_axis_a_tvalid
      .m_axis_result_tdata     (sqrt_to_conv_data ),  // output wire m_axis_result_tdata
      .m_axis_result_tready     (sqrt_to_conv_ready),  // input wire m_axis_result_tready
      .m_axis_result_tvalid      (sqrt_to_conv_valid)    // output wire [31 : 0] m_axis_result_tvalid
    );
    float_to_fixed output_converter (
      .aclk            (aclk),                                  // input wire aclk
      .s_axis_a_tdata          (sqrt_to_conv_data ),            // input wire s_axis_a_tdata
      .s_axis_a_tready          (sqrt_to_conv_ready),            // output wire s_axis_a_tready
      .s_axis_a_tvalid           (sqrt_to_conv_valid),              // input wire [31 : 0] s_axis_a_tvalid
      .m_axis_result_tdata     (out_data_WIRE ),  // output wire m_axis_result_tdata
      .m_axis_result_tready     (out_ready),  // input wire m_axis_result_tready
      .m_axis_result_tvalid      (out_valid)    // output wire [15 : 0] m_axis_result_tvalid
    );


endmodule
