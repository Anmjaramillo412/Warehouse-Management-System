// ============================================================
// MODULE NAVIGATION
// ============================================================

// Set only by openMaterialFromProjection() (see the Projection detail
// section below), right after it switches into the Materials module -
// so showMaterialDetail() knows to offer a "Go Back to Projection"
// button. Reset to null at the top of every openModule() call, so any
// ordinary navigation into Materials (sidebar, Display Materials,
// Search Material, etc.) always starts without a stray back button;
// openMaterialFromProjection sets it again immediately afterward.

let materialsReturnProjectionID = null;

// Set by openMaterialFromInventoryCount(): the Inventory Count whose
// sheet the Back button on the Material detail returns to.
let materialsReturnInventoryCount = null;

// Same idea for Display Products: set only by openMaterialFromProduct()
// so the Material detail offers a "Go Back to Products" button.
let materialsReturnToProducts = false;

// Display Materials / Archived Materials: remembers which list was
// open (and how far it was scrolled) when a Material row was clicked,
// so the single "Back" button on the Material detail returns exactly
// there.
let materialsReturnToList = false;

// Set by modifyMaterialFromDetail(): the Material whose detail view the
// "Back" button on the Modify Material form returns to. Cleared when
// Modify Material is opened from the menu or the module changes.
let materialModifyBackID = null;
let lastMaterialsList = null;

// Supplier chosen in the materials list filter ("" = all); kept when
// coming back to the list from a material
let materialsSupplierFilter = "";
let materialsListScroll = 0;


function openModule(module) {

    materialsReturnProjectionID = null;

    materialsReturnInventoryCount = null;

    materialsReturnToProducts = false;

    materialsReturnToList = false;

    materialModifyBackID = null;

    const dashboard =
        document.querySelector(".dashboard");

    const moduleView =
        document.getElementById("module-view");

    const content =
        document.getElementById("module-content");

    dashboard.style.display = "none";

    moduleView.classList.remove("hidden");

    content.innerHTML = "";


    // ========================================================
    // MATERIAL MANAGER
    // ========================================================

    if (module === "materials") {

        content.innerHTML = `

            <h1>
                Material Manager
            </h1>

            <p>
                Manage materials stored in the system.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateMaterial()">
                    Create Material
                </button>

                <button onclick="materialModifyBackID = null; showModifyMaterial()">
                    Modify Material
                </button>

                <button onclick="showDeleteMaterial()">
                    Delete Material
                </button>

                <button onclick="showSearchMaterial()">
                    Search Material
                </button>

                <button onclick="displayMaterials()">
                    Display Materials
                </button>

                <button class="button-muted" onclick="displayArchivedMaterials()">
                    Archived Materials
                </button>

            </div>

            <div id="material-content">
            </div>
        `;
    }

    // ========================================================
    // PRODUCT MANAGER
    // ========================================================

    else if (module === "products") {

        content.innerHTML = `

            <h1>
                Product Manager
            </h1>

            <p>
                Manage products and their Bills of Materials.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateProduct()">
                    Create Product
                </button>

                <button onclick="showModifyProduct()">
                    Modify Product
                </button>

                <button onclick="showDisplayProducts()">
                    Display Products
                </button>

                <button onclick="showDeleteProduct()">
                    Delete Product
                </button>

                <button onclick="showSellProduct()">
                    Sell Product
                </button>

                <button class="button-accent" onclick="showProductSalesHistory()">
                    Sales History
                </button>

                <button class="button-muted" onclick="showProductSettings()">
                    Settings
                </button>

                <div style="flex-basis: 100%; height: 0; margin-top: -15px;"></div>

                <button onclick="showPurchaseProductCost()">
                    Product Cost
                </button>

                <button onclick="showPurchaseCostByCategory()">
                    Cost by Category
                </button>

                <button onclick="showPurchaseReviewProduct()">
                    Review Product
                </button>

            </div>

            <div id="product-content">
            </div>

            <div id="purchase-content">
            </div>

        `;

        // Product views and cost views live in two containers; only one
        // of them may show at a time, so filling one empties the other
        const productPane = document.getElementById("product-content");

        const costPane = document.getElementById("purchase-content");

        const clearOther = (self, other) =>
            new MutationObserver(() => {
                if (self.childNodes.length > 0) {
                    other.innerHTML = "";
                }
            }).observe(self, { childList: true });

        clearOther(productPane, costPane);

        clearOther(costPane, productPane);
    }

    // ========================================================
    // WAREHOUSE MANAGER
    // ========================================================

    else if (module === "warehouses") {

        content.innerHTML = `

            <h1>
                Warehouse Manager
            </h1>

            <p>
                Manage warehouses and storage locations.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateWarehouse()">
                    Create Warehouse
                </button>

                <button onclick="showDeleteWarehouse()">
                    Delete Warehouse
                </button>

                <button onclick="displayWarehouses()">
                    Display Warehouses
                </button>

            </div>

            <div id="warehouse-content">
            </div>
        `;
    }


    // ========================================================
    // INVENTORY MANAGER
    // ========================================================

    else if (module === "inventory") {

        content.innerHTML = `

            <h1>
                Inventory Manager
            </h1>

            <p>
                Manage material movements and inventory.
            </p>

            <div class="module-buttons">

                <button onclick="showGoodsReceipt()">
                    Goods Receipt
                </button>

                <button onclick="showGoodsIssue()">
                    Goods Issue
                </button>

                <button onclick="showTransferMaterial()">
                    Transfer Material
                </button>

                <button onclick="displayInventory()">
                    Display Inventory
                </button>

                <button onclick="inventoryCheck()">
                    Inventory Check
                </button>

                <button class="button-accent" onclick="showInventoryCounts()">
                    Inventory Count
                </button>

            </div>

            <div id="inventory-content">
            </div>
        `;
    }


    // ========================================================
    // SUPPLIER MANAGER
    // ========================================================

    else if (module === "suppliers") {

        content.innerHTML = `

            <h1>
                Supplier Manager
            </h1>

            <p>
                Manage suppliers referenced by materials.
            </p>

            <div class="module-buttons">

                <button onclick="showCreateSupplier()">
                    Create Supplier
                </button>

                <button onclick="showModifySupplier()">
                    Modify Supplier
                </button>

                <button onclick="displaySuppliers()">
                    Display Suppliers
                </button>

                <button onclick="showDeleteSupplier()">
                    Delete Supplier
                </button>

            </div>

            <div id="supplier-content">
            </div>
        `;
    }

    // ========================================================
    // PROCUREMENT
    // ========================================================

    else if (module === "procurement") {

        content.innerHTML = `

            <h1>
                Procurement
            </h1>

            <p>
                Project production needs and manage purchase orders.
            </p>

            <div class="module-buttons">

                <button onclick="showNewProjection()">
                    New Projection
                </button>

                <button onclick="showProjections()">
                    Open Projections
                </button>

                <button class="button-muted" onclick="showArchivedProjections()">
                    Archived Projections
                </button>

                <button class="button-accent" onclick="showNewProcurementOrder()">
                    New Order
                </button>

                <button class="button-accent" onclick="showProcurementOrders()">
                    Open Orders
                </button>

                <button class="button-accent" onclick="showConfirmedOrders()">
                    Confirmed Orders
                </button>

                <button class="button-muted" onclick="showArchivedOrders()">
                    Archived Orders
                </button>

            </div>

            <div id="procurement-content">
            </div>
        `;
    }

    // ========================================================
    // PURCHASE
    // ========================================================

    // ========================================================
    // QUOTATIONS
    // ========================================================

    else if (module === "prices") {

        content.innerHTML = `

            <h1>
                Pricing
            </h1>

            <p>
                Supplier quotations with price tiers by quantity, pricing
                of received deliveries (landed cost: material cost plus
                customs and freight), invoices and material price
                history. Product Cost and Cost by Category are in the
                Product Manager.
            </p>

            <div class="module-buttons">

                <button class="button-accent" onclick="showNewQuotation()">
                    New Quotation
                </button>

                <button onclick="showQuotations()">
                    Quotations
                </button>

                <button class="button-accent" onclick="showPurchasePending()">
                    Pending Pricing
                </button>

                <button class="button-accent" onclick="showPurchaseAdjust()">
                    Adjust Material Price
                </button>

                <button onclick="showPurchaseInvoices()">
                    Invoices
                </button>

                <button onclick="showPurchaseHistory()">
                    Material Price History
                </button>

                <button class="button-muted" onclick="showPurchaseSettings()">
                    Settings
                </button>

            </div>

            <div id="purchase-content">
            </div>
        `;
    }

    // ========================================================
    // DATA MANAGEMENT
    // ========================================================

    else if (module === "data") {

        content.innerHTML = `

            <h1>
                Data Management
            </h1>

            <p>
                Save and recover system data.
            </p>


            <div class="module-buttons">

                <button onclick="saveData()">
                    Save Data
                </button>

                <button onclick="loadData()">
                    Load Data
                </button>

                <div class="data-logging-option">

                    <label>

                        <input
                            type="checkbox"
                            id="log-data-operations"
                            onchange="setDataLogging()"
                        >

                        Log Save/Load operations

                    </label>

                </div>

                <div class="data-logging-option">

                    <label>

                        <input
                            type="checkbox"
                            id="auto-save-and-load"
                            onchange="toggleAutoSaveAndLoad()"
                        >

                        Auto Save and Load Data

                    </label>

                </div>

            </div>

            <small>
                When on, Material/Supplier/Warehouse/Product data (what
                "Save Data" and "Load Data" above work on) is saved
                automatically after Goods Receipt, Goods Issue, Transfer,
                Procurement receiving, and creating/deleting a Material,
                Supplier, Warehouse or Product - and loaded automatically
                when the server starts, instead of waiting on the
                buttons above. Procurement Orders and Purchase Invoices
                always save themselves immediately either way.
            </small>


            <div id="data-logging-message">
            </div>


            <h2>
                Documents Folder
            </h2>

            <small>
                Root folder (anywhere on this computer, e.g. a synced
                folder) where Procurement documents - Order
                Confirmation and Lieferschein PDFs, attached from
                Procurement Orders - get saved, one subfolder per
                "PRC-######". Set this once; uploads are blocked until
                it is set.
            </small>

            <div class="form-container">

                <label>
                    Folder path
                </label>

                <input
                    type="text"
                    id="documents-folder-path"
                    placeholder="e.g. C:/Users/ajaramillo/4Tex Documents/Procurement"
                >

                <div class="form-actions">
                    <button onclick="saveDocumentsFolder()">
                        Save
                    </button>
                </div>

                <div id="documents-folder-message">
                </div>

            </div>


            <h2>
                Reset Historial Completo
            </h2>

            <small>
                Permanently deletes every Procurement Order, Purchase
                Invoice (and with it, all Material price history) and
                Production Projection, and clears the Movement Log, so
                PRC/INV numbering starts over at PRC-000001 / INV-000001
                and the program looks brand new for testing. Material,
                Supplier, Warehouse and Product data (the Excel file)
                and every setting above are NOT touched. There is no
                undo - use this only to clear out test data, never once
                there is real data you want to keep.
            </small>

            <div class="form-container">

                <label>
                    Type RESET to confirm
                </label>

                <input
                    type="text"
                    id="reset-numbering-confirm"
                    placeholder="RESET"
                >

                <div class="form-actions">
                    <button onclick="resetNumbering()">
                        Delete All History
                    </button>
                </div>

                <div id="reset-numbering-message">
                </div>

            </div>

        `;


        loadDataLoggingState();
        loadAutoSaveAndLoadState();
        loadDocumentsFolderState();
    }
}

// ============================================================
// CREATE SUPPLIER FORM
// ============================================================

function showCreateSupplier() {

    const content =
        document.getElementById("supplier-content");

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Supplier
            </h2>

            <label>
                Name
            </label>

            <input
                type="text"
                id="supplier-name"
                placeholder="Enter Supplier Name"
            >


            <label>
                Address
            </label>

            <input
                type="text"
                id="supplier-address"
                placeholder="Enter Address"
            >


            <label>
                Country
            </label>

            <input
                type="text"
                id="supplier-country"
                placeholder="Enter Country"
            >


            <label>
                Contact Name
            </label>

            <input
                type="text"
                id="supplier-contact-name"
                placeholder="Enter Contact Name"
            >


            <label>
                Contact Email
            </label>

            <input
                type="email"
                id="supplier-contact-email"
                placeholder="Enter Contact Email"
            >


            <label>
                Website
            </label>

            <input
                type="text"
                id="supplier-website"
                placeholder="https://..."
            >


            <label>
                Ordering Method
            </label>

            <div class="checkbox-group">

                <label class="checkbox-label">
                    <input type="checkbox" class="supplier-ordering-method" value="Online">
                    Online
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="supplier-ordering-method" value="Email">
                    Email
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="supplier-ordering-method" value="PO">
                    PO
                </label>

            </div>


            <label>
                Payment Method
            </label>

            <select id="supplier-payment-method">
                <option value="Invoice">Invoice</option>
                <option value="Pre-payment">Pre-payment</option>
                <option value="Online">Online</option>
            </select>


            <label>
                Lead Time (weeks)
            </label>

            <input
                type="number"
                id="supplier-lead-time"
                min="0"
                value="0"
            >


            <label class="checkbox-label">
                <input type="checkbox" id="supplier-is-internal">
                Internal Supplier
            </label>

            <small>
                This Supplier is 4Tex itself (self-manufactured/
                custom-made parts), not a real external vendor.
                Materials assigned to it are excluded from
                Projections and cannot be ordered through a
                Procurement Order.
            </small>


            <div class="form-actions">

                <button onclick="createSupplier()">
                    Create Supplier
                </button>

            </div>


            <div id="supplier-message">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE SUPPLIER
// ============================================================

async function createSupplier() {

    const name =
        document.getElementById(
            "supplier-name"
        ).value.trim();

    const address =
        document.getElementById(
            "supplier-address"
        ).value.trim();

    const country =
        document.getElementById(
            "supplier-country"
        ).value.trim();

    const contactName =
        document.getElementById(
            "supplier-contact-name"
        ).value.trim();

    const contactEmail =
        document.getElementById(
            "supplier-contact-email"
        ).value.trim();

    const website =
        document.getElementById(
            "supplier-website"
        ).value.trim();

    const orderingMethods =
        Array.from(
            document.querySelectorAll(
                ".supplier-ordering-method:checked"
            )
        ).map(checkbox => checkbox.value);

    const paymentMethod =
        document.getElementById(
            "supplier-payment-method"
        ).value;

    const leadTimeWeeks =
        Number(
            document.getElementById(
                "supplier-lead-time"
            ).value
        ) || 0;

    const isInternal =
        document.getElementById(
            "supplier-is-internal"
        ).checked;

    const message =
        document.getElementById(
            "supplier-message"
        );


    if (!name) {

        message.textContent =
            "Supplier name is required.";

        return;
    }


    const supplier = {

        name: name,

        address: address,

        country: country,

        contactName: contactName,

        contactEmail: contactEmail,

        website: website,

        orderingMethods: orderingMethods,

        paymentMethod: paymentMethod,

        leadTimeWeeks: leadTimeWeeks,

        isInternal: isInternal
    };


    try {

        const response =
            await fetch(
                "/api/suppliers/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(supplier)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Supplier created successfully.";

            showCreateSupplier();
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// MODIFY SUPPLIER FORM
// ============================================================

function showModifySupplier(statusMessage) {

    const content =
        document.getElementById(
            "supplier-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Modify Supplier
            </h2>

            ${statusMessage ?
                `<p class="success-message">${escapeHtml(statusMessage)}</p>` :
                ""}

            <label>
                Supplier
            </label>

            <select id="ms-lookup-name">
                <option value="">Loading suppliers...</option>
            </select>

            <div class="form-actions">

                <button onclick="loadSupplierForModify()">
                    Load Supplier
                </button>

            </div>

            <div id="modify-supplier-form">
            </div>

        </div>
    `;

    populateSupplierSelect("ms-lookup-name");
}

// ============================================================
// LOAD SUPPLIER FOR MODIFY
// ============================================================

async function loadSupplierForModify() {

    const name =
        document.getElementById(
            "ms-lookup-name"
        ).value;

    const modifyForm =
        document.getElementById(
            "modify-supplier-form"
        );

    if (!name) {

        modifyForm.innerHTML = `
            <p>
                Please select a supplier.
            </p>
        `;

        return;
    }

    modifyForm.innerHTML = `
        <p>
            Loading supplier...
        </p>
    `;

    try {

        const response =
            await fetch(
                "/api/suppliers/lookup",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({ name: name })
                }
            );

        const data =
            await response.json();

        if (!response.ok) {

            modifyForm.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }

        const orderingMethods =
            data.orderingMethods || [];

        modifyForm.innerHTML = `

            <input type="hidden" id="ms-name" value="${escapeHtml(data.name)}">

            <p>
                Supplier:
                <strong>${escapeHtml(data.name)}</strong>
            </p>

            <label>
                Address
            </label>

            <input type="text" id="ms-address"
                value="${escapeHtml(data.address || "")}">

            <label>
                Country
            </label>

            <input type="text" id="ms-country"
                value="${escapeHtml(data.country || "")}">

            <label>
                Contact Name
            </label>

            <input type="text" id="ms-contact-name"
                value="${escapeHtml(data.contactName || "")}">

            <label>
                Contact Email
            </label>

            <input type="email" id="ms-contact-email"
                value="${escapeHtml(data.contactEmail || "")}">

            <label>
                Website
            </label>

            <input type="text" id="ms-website"
                value="${escapeHtml(data.website || "")}">

            <label>
                Ordering Method
            </label>

            <div class="checkbox-group">

                <label class="checkbox-label">
                    <input type="checkbox" class="ms-ordering-method" value="Online"
                        ${orderingMethods.includes("Online") ? "checked" : ""}>
                    Online
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="ms-ordering-method" value="Email"
                        ${orderingMethods.includes("Email") ? "checked" : ""}>
                    Email
                </label>

                <label class="checkbox-label">
                    <input type="checkbox" class="ms-ordering-method" value="PO"
                        ${orderingMethods.includes("PO") ? "checked" : ""}>
                    PO
                </label>

            </div>

            <label>
                Payment Method
            </label>

            <select id="ms-payment-method">
                <option value="Invoice" ${data.paymentMethod === "Invoice" ? "selected" : ""}>Invoice</option>
                <option value="Pre-payment" ${data.paymentMethod === "Pre-payment" ? "selected" : ""}>Pre-payment</option>
                <option value="Online" ${data.paymentMethod === "Online" ? "selected" : ""}>Online</option>
            </select>

            <label>
                Lead Time (weeks)
            </label>

            <input type="number" id="ms-lead-time" min="0"
                value="${data.leadTimeWeeks || 0}">

            <label class="checkbox-label">
                <input type="checkbox" id="ms-is-internal"
                    ${data.isInternal ? "checked" : ""}>
                Internal Supplier
            </label>

            <small>
                This Supplier is 4Tex itself (self-manufactured/
                custom-made parts), not a real external vendor.
                Materials assigned to it are excluded from
                Projections and cannot be ordered through a
                Procurement Order.
            </small>

            <div class="form-actions">

                <button onclick="modifySupplierSubmit()">
                    Save Changes
                </button>

            </div>

            <div id="modify-supplier-message">
            </div>
        `;
    }
    catch (error) {

        console.error(error);

        modifyForm.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// MODIFY SUPPLIER (SUBMIT)
// ============================================================

async function modifySupplierSubmit() {

    const name =
        document.getElementById(
            "ms-name"
        ).value;

    const address =
        document.getElementById(
            "ms-address"
        ).value.trim();

    const country =
        document.getElementById(
            "ms-country"
        ).value.trim();

    const contactName =
        document.getElementById(
            "ms-contact-name"
        ).value.trim();

    const contactEmail =
        document.getElementById(
            "ms-contact-email"
        ).value.trim();

    const website =
        document.getElementById(
            "ms-website"
        ).value.trim();

    const orderingMethods =
        Array.from(
            document.querySelectorAll(
                ".ms-ordering-method:checked"
            )
        ).map(checkbox => checkbox.value);

    const paymentMethod =
        document.getElementById(
            "ms-payment-method"
        ).value;

    const leadTimeWeeks =
        Number(
            document.getElementById(
                "ms-lead-time"
            ).value
        ) || 0;

    const isInternal =
        document.getElementById(
            "ms-is-internal"
        ).checked;

    const message =
        document.getElementById(
            "modify-supplier-message"
        );

    const supplier = {

        name: name,

        address: address,

        country: country,

        contactName: contactName,

        contactEmail: contactEmail,

        website: website,

        orderingMethods: orderingMethods,

        paymentMethod: paymentMethod,

        leadTimeWeeks: leadTimeWeeks,

        isInternal: isInternal
    };

    try {

        const response =
            await fetch(
                "/api/suppliers/modify",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(supplier)
                }
            );

        const responseText =
            await response.text();

        if (response.ok) {

            // Re-render the form empty (dropdown reset, no fields
            // filled in) so it is visually obvious the change was
            // saved, rather than leaving the just-saved values on
            // screen looking unchanged.
            showModifySupplier(
                "Supplier successfully modified.");

            return;
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY SUPPLIERS
// ============================================================

async function displaySuppliers() {

    const content =
        document.getElementById(
            "supplier-content"
        );

    content.innerHTML = `
        <p>Loading suppliers...</p>
    `;


    try {

        const response =
            await fetch("/api/suppliers");


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.suppliers ||
            data.suppliers.length === 0
        ) {

            content.innerHTML = `

                <div class="empty-message">

                    No suppliers available.

                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Suppliers
                </h2>

                <input type="text" class="product-cost-search"
                    placeholder="Search supplier (name, country, contact, email...)"
                    oninput="filterSupplierRows(this)">

                <small class="supplier-search-count"></small>

                <table class="material-table">

                    <thead>

                        <tr>

                            <th>Name</th>
                            <th>Country</th>
                            <th>Contact</th>
                            <th>Email</th>
                            <th>Website</th>
                            <th>Ordering</th>
                            <th>Payment</th>
                            <th>Lead Time</th>
                            <th>Internal</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (const supplier of data.suppliers) {

            html += `

                <tr>

                    <td>${escapeHtml(supplier.name)}</td>
                    <td>${escapeHtml(supplier.country || "")}</td>
                    <td>${escapeHtml(supplier.contactName || "")}</td>
                    <td>${escapeHtml(supplier.contactEmail || "")}</td>
                    <td>${escapeHtml(supplier.website || "")}</td>
                    <td>${escapeHtml((supplier.orderingMethods || []).join(", "))}</td>
                    <td>${escapeHtml(supplier.paymentMethod || "")}</td>
                    <td>${supplier.leadTimeWeeks} week(s)</td>
                    <td>${supplier.isInternal ? "Yes" : "No"}</td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

        enableColumnFilters(content);

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// Search box of the supplier list: hides the rows that do not match
// any of their cells (name, country, contact, email, ordering...)
function filterSupplierRows(input) {

    const text = input.value.trim().toLowerCase();

    const key = normalizeMaterialKey(text);

    const container = input.closest(".material-table-container");

    const rows = container.querySelectorAll("tbody tr");

    let shown = 0;

    for (const row of rows) {

        const haystack = row.textContent.toLowerCase();

        const match = text === "" ||
            haystack.includes(text) ||
            (key !== "" && normalizeMaterialKey(haystack).includes(key));

        row.dataset.searchHidden = match ? "" : "1";

        applyRowVisibility(row);

        if (match) {
            shown++;
        }
    }

    const count = container.querySelector(".supplier-search-count");

    if (count) {
        count.textContent = text === "" ? "" :
            `${shown} of ${rows.length} suppliers`;
    }
}

// ============================================================
// DELETE SUPPLIER FORM
// ============================================================

function showDeleteSupplier() {

    const content =
        document.getElementById(
            "supplier-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Supplier
            </h2>

            <label>
                Supplier
            </label>

            <select id="delete-supplier-name">
                <option value="">Loading suppliers...</option>
            </select>

            <div class="form-actions">

                <button onclick="deleteSupplier()">
                    Delete Supplier
                </button>

            </div>

            <div id="delete-supplier-message">
            </div>

        </div>
    `;

    populateSupplierSelect("delete-supplier-name");
}

// ============================================================
// DELETE SUPPLIER
// ============================================================

async function deleteSupplier() {

    const name =
        document.getElementById(
            "delete-supplier-name"
        ).value;

    const message =
        document.getElementById(
            "delete-supplier-message"
        );


    if (!name) {

        message.textContent =
            "Please select a supplier.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/suppliers/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({ name: name })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Supplier deleted successfully.";

            showDeleteSupplier();
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// LOAD DATA LOGGING STATE
// ============================================================

async function loadDataLoggingState()
{
    try
    {
        const response =
            await fetch(
                "/api/data/logging"
            );


        const data =
            await response.json();


        if (!response.ok)
        {
            return;
        }


        const checkbox =
            document.getElementById(
                "log-data-operations"
            );


        if (checkbox)
        {
            checkbox.checked =
                data.enabled;
        }
    }
    catch (error)
    {
        console.error(
            "Could not load logging state:",
            error
        );
    }
}

// ============================================================
// SAVE DATA
// ============================================================

async function saveData() {

    const message =
            document.getElementById(
                "data-logging-message"
            );

    try {

        const response =
            await fetch(
                "/api/data/save",
                {
                    method: "POST"
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "✓ Data saved successfully.";

        }
        else {

            message.textContent =
                "Error saving data: "
                + responseText;

        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// LOAD DATA
// ============================================================

async function loadData() {
    
    const message =
        document.getElementById(
            "data-logging-message"
        );

    try {

        const response =
            await fetch(
                "/api/data/load",
                {
                    method: "POST"
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {
            message.textContent =
                "✓ Data loaded successfully.";

        }
        else {

            message.textContent =
                "Error loading data: "
                + responseText;

        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// CREATE MATERIAL FORM
// ============================================================

function showCreateMaterial() {

    const content =
        document.getElementById("material-content");

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Material
            </h2>

            <label>
                Material ID
            </label>

            <input
                type="text"
                id="material-id"
                placeholder="###-###### or ######-00"
                maxlength="10"
            >

            <small>
                Format: ###-###### or ######-00
            </small>


            <label>
                Material Name
            </label>

            <input
                type="text"
                id="material-name"
                placeholder="Enter Material Name"
            >


            <label>
                Description
            </label>

            <textarea
                id="material-description"
                placeholder="Enter Material Description"
                rows="4"></textarea>


            <label>
                Unit of Measure
            </label>

            <input
                type="text"
                id="material-uom"
                value="pcs"
                placeholder="pcs"
            >


            <label>
                Category (optional)
            </label>

            <input
                type="text"
                id="material-category"
                placeholder="Enter Category (optional)"
            >


            <label>
                Material Type
            </label>

            <select
                id="material-type"
                onchange="toggleDrawingNumberField('material')">

                <option value="Standard Part">Standard Part</option>
                <option value="Design Part">Design Part</option>
                <option value="PCB">PCB</option>

            </select>


            <div id="material-drawing-number-group" class="hidden">

                <label>
                    Drawing Number
                </label>

                <input
                    type="text"
                    id="material-drawing-number"
                    placeholder="e.g. 123-ASM-4567"
                >

                <small>
                    Format: ###-ASM-####, ###-PAR-####,
                    I-BU#-##, OWI-BU#-##, or ###-PCB-####
                </small>

                <label>
                    Drawing Version
                </label>

                <input
                    type="text"
                    id="material-drawing-version"
                    placeholder="e.g. AA, BA, or 1"
                >

                <small>
                    A number (1, 2, ...) or two letters (AA, BA, ...)
                </small>

            </div>


            <div id="material-manufacturer-fields-group">

                <label>
                    Manufacturer
                </label>

                <input
                    type="text"
                    id="material-manufacturer"
                    placeholder="Enter Manufacturer"
                >


                <label>
                    Manufacturer Part Number
                </label>

                <input
                    type="text"
                    id="material-manufacturer-pn"
                    placeholder="Enter Manufacturer Part Number"
                >

            </div>


            <div id="material-supplier-fields-group">

                <!-- Kept (hidden) for the rest of the code: the
                     supplier marked "Primary" in the list below is
                     copied into these fields on save. -->
                <div style="display: none;">

                <label>
                    Primary Supplier
                </label>

                <div class="combobox">

                    <input
                        type="text"
                        id="material-supplier-search"
                        autocomplete="off"
                        placeholder="Search supplier by name..."
                        oninput="handleSupplierSearchInput('material')"
                        onfocus="renderSupplierOptions('material')"
                        onblur="hideSupplierOptionsDelayed('material')"
                    >

                    <input type="hidden" id="material-supplier">

                    <div id="material-supplier-options"
                        class="combobox-options hidden">
                    </div>

                </div>

                <div id="material-supplier-pn-group">

                    <input
                        type="text"
                        id="material-supplier-pn"
                    >

                </div>

                </div>


                <label>
                    Suppliers
                </label>

                <small>
                    Add every supplier this Material can be purchased
                    from and tick <strong>Primary</strong> on the main
                    one. To change the primary supplier later, just
                    tick another row.
                    No supplier listed?
                    <a href="#" onclick="openModule('suppliers'); return false;">
                        Create one first.
                    </a>
                </small>

                <div id="material-addsup-lines">
                </div>

                <button
                    type="button"
                    onclick="addAdditionalSupplierRow('material')">

                    + Add Supplier

                </button>

            </div>


            <label>
                Material Photo
            </label>

            <input
                type="file"
                id="material-photo"
                accept="image/*"
            >

            <label class="checkbox-label">

                <input
                    type="checkbox"
                    id="material-active"
                    checked
                >

                Active Material

            </label>


            <div class="form-actions">

                <button
                    onclick="createMaterial()">

                    Create Material

                </button>

            </div>


            <div id="material-message">
            </div>

        </div>
    `;

    initSupplierCombobox("material");

    additionalSupplierCounters["material"] = 0;

    // First row of the supplier list (marked Primary)
    addAdditionalSupplierRow("material", "", "", true);
}

// ============================================================
// TOGGLE DRAWING NUMBER FIELD
// ============================================================
// Drawing Number only applies to Design Part / PCB materials.

function toggleDrawingNumberField(prefix) {

    const type =
        document.getElementById(
            prefix + "-type"
        ).value;

    const drawingGroup =
        document.getElementById(
            prefix + "-drawing-number-group"
        );

    const manufacturerGroup =
        document.getElementById(
            prefix + "-manufacturer-fields-group"
        );

    const supplierPnGroup =
        document.getElementById(
            prefix + "-supplier-pn-group"
        );

    const requiresDrawing =
        (type === "Design Part" || type === "PCB");

    drawingGroup.classList.toggle(
        "hidden",
        !requiresDrawing);

    // A Design Part / PCB material is custom-made: it has no
    // Manufacturer, Manufacturer Part Number or Supplier Part
    // Number (nothing here has a commercial part number) - those
    // only apply to a Standard Part, so this group toggles the
    // opposite way from the drawing fields above. The Supplier
    // itself (just the name, who makes/will make the part) still
    // applies to every Material type, so its group is never hidden
    // here - only its Part Number sub-field is.
    if (manufacturerGroup) {

        manufacturerGroup.classList.toggle(
            "hidden",
            requiresDrawing);
    }

    if (supplierPnGroup) {

        supplierPnGroup.classList.toggle(
            "hidden",
            requiresDrawing);
    }

    // Additional Suppliers rows also have their own Supplier Part
    // Number input per row - hide those the same way.
    document.querySelectorAll(
        `[id^="${prefix}-addsup-"][id$="-pn"]`
    ).forEach(input => {

        input.classList.toggle(
            "hidden",
            requiresDrawing);
    });
}

// ============================================================
// POPULATE SUPPLIER SELECT
// ============================================================

async function populateSupplierSelect(selectId, selectedName) {

    const select =
        document.getElementById(selectId);

    if (!select) {
        return;
    }

    try {

        const response =
            await fetch("/api/suppliers");

        const data =
            await response.json();

        const suppliers =
            data.suppliers || [];

        if (suppliers.length === 0) {

            select.innerHTML = `
                <option value="">
                    No suppliers available
                </option>
            `;

            return;
        }

        select.innerHTML =
            `<option value="">Select a supplier</option>` +
            suppliers.map(supplier => `
                <option value="${escapeHtml(supplier.name)}">
                    ${escapeHtml(supplier.name)}
                </option>
            `).join("");

        if (selectedName) {
            select.value = selectedName;
        }
    }
    catch (error) {

        console.error(error);

        select.innerHTML = `
            <option value="">
                Could not load suppliers
            </option>
        `;
    }
}

// Same pattern as populateSupplierSelect() above, for a Product's
// Main Warehouse (Create Product / Modify Product) - see
// showDisplayProducts() for where that choice ends up being used, to
// show only that one Warehouse's stock instead of every Warehouse.
// "" (empty selection) means "none set", sent to the backend as 0.

// Options used by the Warehouse dropdowns of the Inventory Manager forms
const WH_OPTS = { placeholder: "Select Warehouse", showID: true };

async function populateWarehouseSelect(selectId, selectedID, options) {

    // options (optional): { placeholder, showID, autoSelectSingle }
    const placeholder =
        (options && options.placeholder) || "No Main Warehouse";


    const select =
        document.getElementById(selectId);

    if (!select) {
        return;
    }

    try {

        const response =
            await fetch("/api/warehouses");

        const data =
            await response.json();

        const warehouses =
            data.warehouses || [];

        if (warehouses.length === 0) {

            select.innerHTML = `
                <option value="">
                    No warehouses available
                </option>
            `;

            return;
        }

        select.innerHTML =
            `<option value="">${escapeHtml(placeholder)}</option>` +
            warehouses.map(warehouse => `
                <option value="${warehouse.id}">
                    ${options && options.showID
                        ? escapeHtml(warehouse.id + " - ")
                        : ""}${escapeHtml(warehouse.name)}
                </option>
            `).join("");

        if (selectedID) {
            select.value = String(selectedID);
        }
        else if (options && options.autoSelectSingle &&
                 warehouses.length === 1) {
            select.value = String(warehouses[0].id);
        }
    }
    catch (error) {

        console.error(error);

        select.innerHTML = `
            <option value="">
                Could not load warehouses
            </option>
        `;
    }
}

// ============================================================
// SUPPLIER SEARCH COMBOBOX
// ============================================================
// Used by the Material create/modify forms to search for an
// existing supplier by name instead of picking from a long list.
// The hidden "<prefix>-supplier" input only ever holds a name
// that exactly matches an existing supplier - it is cleared
// whenever the visible search text does not match one.

let supplierCache = null;

async function loadSupplierCache(forceRefresh) {

    if (supplierCache && !forceRefresh) {
        return supplierCache;
    }

    try {

        const response =
            await fetch("/api/suppliers");

        const data =
            await response.json();

        supplierCache =
            data.suppliers || [];
    }
    catch (error) {

        console.error(error);

        supplierCache = [];
    }

    return supplierCache;
}

async function initSupplierCombobox(prefix, selectedName) {

    await loadSupplierCache(true);

    const hiddenInput =
        document.getElementById(prefix + "-supplier");

    const searchInput =
        document.getElementById(prefix + "-supplier-search");

    if (!hiddenInput || !searchInput) {
        return;
    }

    hiddenInput.value = selectedName || "";
    searchInput.value = selectedName || "";
}

function renderSupplierOptions(prefix) {

    const searchInput =
        document.getElementById(prefix + "-supplier-search");

    const optionsBox =
        document.getElementById(prefix + "-supplier-options");

    if (!searchInput || !optionsBox) {
        return;
    }

    const filterText =
        searchInput.value.trim().toLowerCase();

    const suppliers =
        supplierCache || [];

    const matches =
        suppliers.filter(supplier =>
            supplier.name.toLowerCase().includes(filterText)
        );

    if (matches.length === 0) {

        optionsBox.innerHTML = `
            <div class="combobox-option combobox-empty">
                No suppliers found
            </div>
        `;
    }
    else {

        optionsBox.innerHTML =
            matches.map(supplier => `
                <div class="combobox-option"
                    onmousedown="selectSupplierOption('${prefix}', '${escapeHtml(supplier.name)}')">
                    ${escapeHtml(supplier.name)}${
                        supplier.isInternal
                            ? " (Internal - no Projections/Procurement)"
                            : ""
                    }
                </div>
            `).join("");
    }

    optionsBox.classList.remove("hidden");
}

function handleSupplierSearchInput(prefix) {

    renderSupplierOptions(prefix);

    const searchInput =
        document.getElementById(prefix + "-supplier-search");

    const hiddenInput =
        document.getElementById(prefix + "-supplier");

    const suppliers =
        supplierCache || [];

    const typedName =
        searchInput.value.trim();

    const exactMatch =
        suppliers.find(supplier =>
            supplier.name === typedName
        );

    hiddenInput.value =
        exactMatch ? exactMatch.name : "";
}

function selectSupplierOption(prefix, name) {

    document.getElementById(
        prefix + "-supplier"
    ).value = name;

    document.getElementById(
        prefix + "-supplier-search"
    ).value = name;

    document.getElementById(
        prefix + "-supplier-options"
    ).classList.add("hidden");

    // If this combobox belongs to an Other Cost row, its quantity
    // hint depends on which Supplier is picked - refresh it now
    // rather than leaving the earlier Supplier's (or "Select a
    // Supplier") number showing.
    if (typeof otherCostRowSources !== "undefined" &&
        otherCostRowSources[prefix]) {

        refreshOtherCostQuantity(prefix);
    }
}

function hideSupplierOptionsDelayed(prefix) {

    // Delay so a click (onmousedown) on an option
    // still registers before the dropdown disappears.

    setTimeout(() => {

        const optionsBox =
            document.getElementById(prefix + "-supplier-options");

        if (optionsBox) {
            optionsBox.classList.add("hidden");
        }

    }, 150);
}

// ============================================================
// SUPPLIER COMBOBOX MARKUP HELPER
// ============================================================
// Same combobox markup used inline for the primary Supplier field,
// factored out so each Additional Supplier row can build one too.

function supplierComboboxHtml(prefix, placeholder) {

    return `
        <div class="combobox">

            <input
                type="text"
                id="${prefix}-supplier-search"
                autocomplete="off"
                placeholder="${placeholder || "Search supplier by name..."}"
                oninput="handleSupplierSearchInput('${prefix}')"
                onfocus="renderSupplierOptions('${prefix}')"
                onblur="hideSupplierOptionsDelayed('${prefix}')"
            >

            <input type="hidden" id="${prefix}-supplier">

            <div id="${prefix}-supplier-options"
                class="combobox-options hidden">
            </div>

        </div>
    `;
}

// ============================================================
// ADDITIONAL SUPPLIERS - REPEATABLE ROWS (Create/Modify Material)
// ============================================================
// A Material can be purchased from several Suppliers, but only one
// (the "Primary Supplier" field above) is the main one. These rows
// hold the rest - each with its own Supplier (via the same combobox
// pattern as everywhere else) and its own Supplier Part Number.

let additionalSupplierCounters = {};

function additionalSupplierRowHtml(
    formPrefix,
    rowIndex,
    selectedName,
    partNumber) {

    const rowPrefix =
        `${formPrefix}-addsup-${rowIndex}`;

    return `
        <div class="bom-row" id="${rowPrefix}-row">

            ${supplierComboboxHtml(rowPrefix, "Search supplier by name...")}

            <input
                type="text"
                id="${rowPrefix}-pn"
                placeholder="Supplier Part Number"
                value="${escapeHtml(partNumber || "")}"
            >

            <label class="checkbox-label" style="margin: 8px 0 !important;" title="Main supplier of this Material">

                <input
                    type="radio"
                    name="${formPrefix}-primary-supplier"
                    id="${rowPrefix}-primary"
                >

                Primary

            </label>

            <button
                type="button"
                onclick="removeSupplierRow('${formPrefix}', '${rowPrefix}')">

                Remove

            </button>

        </div>
    `;
}

function addAdditionalSupplierRow(
    formPrefix,
    selectedName,
    partNumber,
    isPrimary) {

    const container =
        document.getElementById(
            `${formPrefix}-addsup-lines`
        );

    if (!container) {
        return;
    }

    if (!(formPrefix in additionalSupplierCounters)) {
        additionalSupplierCounters[formPrefix] = 0;
    }

    const rowIndex =
        additionalSupplierCounters[formPrefix]++;

    const wrapper =
        document.createElement("div");

    wrapper.innerHTML =
        additionalSupplierRowHtml(
            formPrefix,
            rowIndex,
            selectedName,
            partNumber
        ).trim();

    container.appendChild(
        wrapper.firstElementChild
    );

    initSupplierCombobox(
        `${formPrefix}-addsup-${rowIndex}`,
        selectedName
    );

    // Exactly one row is always the primary one: the requested one,
    // or the first row if none is marked yet
    const radios = document.querySelectorAll(
        `input[name="${formPrefix}-primary-supplier"]`);

    const newRadio =
        document.getElementById(`${formPrefix}-addsup-${rowIndex}-primary`);

    if (newRadio && (isPrimary ||
        ![...radios].some(radio => radio.checked))) {

        newRadio.checked = true;
    }

    // A custom-made Material (Design Part / PCB) has no commercial
    // part number at all - if that's the current Material Type,
    // hide this new row's Supplier Part Number input too, same as
    // the primary one.

    const typeSelect =
        document.getElementById(`${formPrefix}-type`);

    const requiresDrawing =
        typeSelect &&
        (typeSelect.value === "Design Part" ||
            typeSelect.value === "PCB");

    if (requiresDrawing) {

        const pnInput =
            document.getElementById(`${formPrefix}-addsup-${rowIndex}-pn`);

        if (pnInput) {

            pnInput.classList.add("hidden");
        }
    }
}

// Reads every Additional Supplier row currently in the form back
// into a plain [{ supplier, supplierPartNumber }, ...] array, ready
// to send to the backend. Rows left empty (never given a Supplier)
// are skipped rather than sent as blank entries.

// Removing the row marked Primary hands the mark to the first
// remaining row.
function removeSupplierRow(formPrefix, rowPrefix) {

    const row = document.getElementById(`${rowPrefix}-row`);

    if (!row) {
        return;
    }

    row.remove();

    const radios = [...document.querySelectorAll(
        `input[name="${formPrefix}-primary-supplier"]`)];

    if (radios.length > 0 && !radios.some(radio => radio.checked)) {

        radios[0].checked = true;
    }
}

// Copies the supplier marked Primary in the list into the (hidden)
// primary-supplier fields the rest of the form reads, and flags that
// row so collectAdditionalSuppliers() leaves it out. A marked row
// with no supplier chosen falls back to the first row that has one.
function syncSupplierList(formPrefix) {

    const rows = [...document.querySelectorAll(
        `[id^="${formPrefix}-addsup-"][id$="-row"]`)];

    const supplierOf = row => {

        const hidden = row.querySelector(
            'input[type="hidden"][id$="-supplier"]');

        return hidden ? hidden.value.trim() : "";
    };

    let primary = rows.find(row => {

        const radio = row.querySelector('input[type="radio"]');

        return radio && radio.checked && supplierOf(row);
    });

    if (!primary) {

        primary = rows.find(row => supplierOf(row));
    }

    rows.forEach(row => delete row.dataset.isPrimary);

    const hiddenSupplier =
        document.getElementById(`${formPrefix}-supplier`);

    const searchSupplier =
        document.getElementById(`${formPrefix}-supplier-search`);

    const primaryPn =
        document.getElementById(`${formPrefix}-supplier-pn`);

    if (!primary) {

        if (hiddenSupplier) hiddenSupplier.value = "";

        if (searchSupplier) searchSupplier.value = "";

        if (primaryPn) primaryPn.value = "";

        return;
    }

    primary.dataset.isPrimary = "1";

    const pnInput = primary.querySelector('input[id$="-pn"]');

    const searchInput = primary.querySelector(
        'input[type="text"][id$="-supplier-search"]');

    if (hiddenSupplier) hiddenSupplier.value = supplierOf(primary);

    if (searchSupplier) {
        searchSupplier.value = searchInput ? searchInput.value : "";
    }

    if (primaryPn) primaryPn.value = pnInput ? pnInput.value.trim() : "";
}

function collectAdditionalSuppliers(formPrefix) {

    syncSupplierList(formPrefix);

    const rows =
        document.querySelectorAll(
            `[id^="${formPrefix}-addsup-"][id$="-row"]`
        );

    const result = [];

    for (const row of rows) {

        // The supplier marked Primary is sent as the primary one
        if (row.dataset.isPrimary === "1") {
            continue;
        }

        const hiddenInput =
            row.querySelector(
                'input[type="hidden"][id$="-supplier"]'
            );

        const supplierName =
            hiddenInput ? hiddenInput.value.trim() : "";

        if (!supplierName) {
            continue;
        }

        const partNumberInput =
            row.querySelector('input[id$="-pn"]');

        result.push({
            supplier: supplierName,
            supplierPartNumber:
                partNumberInput ?
                partNumberInput.value.trim() : ""
        });
    }

    return result;
}

// ============================================================
// MATERIAL SEARCH COMBOBOX
// ============================================================
// Generic Material ID lookup, used anywhere a Material ID needs
// to be entered (Search/Delete/Modify Material, Transfer Material,
// Goods Receipt/Issue line items), so materials can be found by
// typing part of the ID or name instead of remembering the exact
// ID. The hidden "<prefix>-material-id" input only ever holds an
// ID that exactly matches an existing material - it is cleared
// whenever the visible search text does not match one exactly.

let materialCache = null;

let materialCacheLoading = null;

// Materials always read in Material ID order (e.g. every 008-...
// before the 017-... ones), not in the order they were created.
// IDs are fixed-format "NNN-NNNNNN", so a numeric-aware text
// comparison sorts them correctly. Returns a new, sorted array.

function sortMaterialsByID(materials) {

    return [...materials].sort((a, b) =>
        String(a.id).localeCompare(
            String(b.id),
            undefined,
            { numeric: true }
        )
    );
}

async function loadMaterialCache(forceRefresh) {

    if (materialCache && !forceRefresh) {
        return materialCache;
    }

    // Several comboboxes can be initialised at the same moment (e.g.
    // a form with many rows) - share one request instead of firing
    // one per box.

    if (materialCacheLoading) {
        return materialCacheLoading;
    }

    materialCacheLoading = fetchMaterialCache();

    try {
        return await materialCacheLoading;
    }
    finally {
        materialCacheLoading = null;
    }
}

async function fetchMaterialCache() {

    try {

        const response =
            await fetch("/api/materials");

        const data =
            await response.json();

        materialCache =
            sortMaterialsByID(data.materials || []);
    }
    catch (error) {

        console.error(error);

        materialCache = [];
    }

    return materialCache;
}

async function initMaterialCombobox(prefix, selectedID) {

    // Always re-read the Material list when a combobox is opened:
    // a Material created (or renamed/deleted) after the page was
    // loaded would otherwise never show up in the search until a
    // manual page reload.

    await loadMaterialCache(true);

    const hiddenInput =
        document.getElementById(prefix + "-material-id");

    const searchInput =
        document.getElementById(prefix + "-material-id-search");

    if (!hiddenInput || !searchInput) {
        return;
    }

    // A row added empty ("+ Add Component") must not be wiped if the
    // user already started typing / picked a Material while the list
    // above was still loading.

    if (!selectedID &&
        (searchInput.value || hiddenInput.value)) {

        return;
    }

    hiddenInput.value = selectedID || "";

    // Show the full "ID - Name" label, the same as the dropdown
    // option itself, instead of just the bare ID - much easier to
    // recognize a Material at a glance than its ID alone (see
    // selectMaterialOption(), which fills this in the same way right
    // after a fresh selection).

    searchInput.value =
        selectedID ?
        materialComboboxLabel(selectedID) : "";

    updateMaterialPartNumberHint(prefix, selectedID || "");
}

// Optional: a form that has an element "<prefix>-material-id-pn" next
// to its Material box gets the selected material's supplier part
// number(s) there - the chosen Supplier's first (quotations).
function updateMaterialPartNumberHint(prefix, materialID) {

    const hint =
        document.getElementById(prefix + "-material-id-pn");

    if (!hint) {
        return;
    }

    const material = materialID ?
        (materialCache || []).find(m => m.id === materialID) : null;

    if (!material) {

        hint.innerHTML = "";

        return;
    }

    const preferred = materialComboboxPreferredSupplier(prefix);

    const links = materialPartNumbers(material)
        .filter(link => link.kind === "supplier")
        .sort((a, b) =>
            (b.supplier === preferred ? 1 : 0) -
            (a.supplier === preferred ? 1 : 0));

    if (links.length === 0) {

        hint.innerHTML =
            `<small style="color:#b45309;">No supplier part number saved for this material</small>`;

        return;
    }

    hint.innerHTML = links.map((link, index) =>
        `<span style="${index === 0 ? "font-weight:bold; color:#1f2937;" : "color:#6b7280;"}">` +
        `Supplier part # ${escapeHtml(link.pn)}` +
        (link.supplier ? ` &middot; ${escapeHtml(link.supplier)}` : "") +
        `</span>`).join("<br>");
}

// Returns "ID - Name" for a Material already in materialCache, or
// just the ID if it can't be found there (e.g. cache not loaded yet,
// or the Material was since deleted) - shared by initMaterialCombobox
// and selectMaterialOption so both fill the search box the same way.

function materialComboboxLabel(materialID) {

    const material =
        (materialCache || []).find(
            m => m.id === materialID
        );

    return material ?
        `${material.id} — ${material.name}` :
        materialID;
}

// Part numbers are compared ignoring case, hyphens, spaces, dots and
// slashes ("5001.3856.030" = "5001 3856-030")
function normalizeMaterialKey(text) {

    return String(text || "").toLowerCase().replace(/[^a-z0-9]/g, "");
}

// Supplier a form wants its materials matched against first: the
// quotation lines use the Supplier chosen at the top of the form
function materialComboboxPreferredSupplier(prefix) {

    if (prefix.startsWith("quo-line-")) {

        const select = document.getElementById("quo-supplier");

        return select ? select.value : "";
    }

    return "";
}

// All part numbers of a material: [{ pn, supplier, kind }]
function materialPartNumbers(material) {

    const list = [];

    if (material.supplierPartNumber) {
        list.push({ pn: material.supplierPartNumber,
            supplier: material.supplier || "", kind: "supplier" });
    }

    for (const link of (material.additionalSuppliers || [])) {

        if (link.supplierPartNumber) {
            list.push({ pn: link.supplierPartNumber,
                supplier: link.supplier || "", kind: "supplier" });
        }
    }

    if (material.manufacturerPartNumber) {
        list.push({ pn: material.manufacturerPartNumber,
            supplier: material.manufacturer || "", kind: "manufacturer" });
    }

    return list;
}

// Scores one material against what was typed: null = no match.
// Exact part number > exact ID > part number / ID start > name or
// part number contains. Materials of the preferred Supplier come first.
function matchMaterialForSearch(material, text, preferredSupplier) {

    const plain = text.toLowerCase();

    const key = normalizeMaterialKey(text);

    const supplierLinks = materialPartNumbers(material);

    const supplierMaterial = !!preferredSupplier && (
        material.supplier === preferredSupplier ||
        (material.additionalSuppliers || []).some(
            link => link.supplier === preferredSupplier));

    let score = null;

    let matchedPN = null;

    if (!plain) {

        score = 0;
    }
    else {

        const idKey = normalizeMaterialKey(material.id);

        if (key && idKey === key) {
            score = 90;
        }
        else if (key && idKey.startsWith(key)) {
            score = 55;
        }
        else if (key && idKey.includes(key)) {
            score = 40;
        }

        if ((material.name || "").toLowerCase().includes(plain)) {
            score = Math.max(score === null ? 0 : score, 40);
        }

        for (const link of supplierLinks) {

            const pnKey = normalizeMaterialKey(link.pn);

            let pnScore = null;

            if (key.length >= 3 && pnKey === key) {
                pnScore = 100;
            }
            else if (key.length >= 3 && pnKey.startsWith(key)) {
                pnScore = 60;
            }
            else if (key.length >= 3 && pnKey.includes(key)) {
                pnScore = 35;
            }

            if (pnScore !== null && (score === null || pnScore > score ||
                matchedPN === null)) {

                if (score === null || pnScore >= score) {
                    matchedPN = link;
                }

                score = Math.max(score === null ? 0 : score, pnScore);
            }
        }
    }

    if (score === null) {
        return null;
    }

    return {
        material: material,
        score: score + (supplierMaterial ? 20 : 0),
        supplierMaterial: supplierMaterial,
        matchedPN: matchedPN
    };
}

function findMaterialMatches(prefix, text) {

    const preferred = materialComboboxPreferredSupplier(prefix);

    return (materialCache || [])
        .map(material => matchMaterialForSearch(material, text.trim(), preferred))
        .filter(match => match)
        .sort((x, y) => y.score - x.score ||
            x.material.id.localeCompare(y.material.id));
}

function renderMaterialOptions(prefix) {

    const searchInput =
        document.getElementById(prefix + "-material-id-search");

    const optionsBox =
        document.getElementById(prefix + "-material-id-options");

    if (!searchInput || !optionsBox) {
        return;
    }

    const preferred =
        materialComboboxPreferredSupplier(prefix);

    const matches =
        findMaterialMatches(prefix, searchInput.value);

    if (matches.length === 0) {

        optionsBox.innerHTML = `
            <div class="combobox-option combobox-empty">
                No materials found
            </div>
        `;
    }
    else {

        const limited =
            matches.slice(0, 30);

        optionsBox.innerHTML =
            limited.map(match => {

                const material = match.material;

                // part number shown: the one that matched, else the
                // preferred Supplier's own
                let link = match.matchedPN;

                if (!link && preferred) {

                    link = materialPartNumbers(material).find(
                        entry => entry.kind === "supplier" &&
                            entry.supplier === preferred) || null;
                }

                return `
                <div class="combobox-option"
                    onmousedown="selectMaterialOption('${prefix}', '${escapeHtml(material.id)}')">
                    <strong>${escapeHtml(material.id)}</strong>
                    &nbsp;&mdash;&nbsp;${escapeHtml(material.name)}
                    ${link ?
                        `<br><small style="color:#6b7280;">${
                            link.kind === "manufacturer" ? "Mfr" : "Part #"
                        } ${escapeHtml(link.pn)}${
                            link.supplier ? " &middot; " + escapeHtml(link.supplier) : ""
                        }</small>` : ""}
                </div>`;
            }).join("") +
            (matches.length > limited.length
                ? `<div class="combobox-option combobox-empty">
                        ${matches.length - limited.length} more - keep typing to narrow down
                   </div>`
                : "");
    }

    optionsBox.classList.remove("hidden");
}

function handleMaterialSearchInput(prefix) {

    renderMaterialOptions(prefix);

    const searchInput =
        document.getElementById(prefix + "-material-id-search");

    const hiddenInput =
        document.getElementById(prefix + "-material-id");

    const materials =
        materialCache || [];

    const typedText =
        searchInput.value.trim();

    const exactMatch =
        materials.find(material =>
            material.id === typedText
        );

    hiddenInput.value =
        exactMatch ? exactMatch.id : "";

    // A part number typed in full (any format) that points to ONE
    // material selects it right away
    if (!exactMatch && normalizeMaterialKey(typedText).length >= 4) {

        const exactPN = findMaterialMatches(prefix, typedText)
            .filter(match => match.score >= 100);

        const best = exactPN.length > 0 ? exactPN[0] : null;

        const unique = best && (
            exactPN.length === 1 ||
            (best.supplierMaterial &&
                exactPN.filter(match => match.supplierMaterial).length === 1));

        if (unique) {

            selectMaterialOption(prefix, best.material.id);

            return;
        }
    }

    updateMaterialPartNumberHint(prefix, hiddenInput.value);

    updateMaterialCurrentPriceHint(prefix, hiddenInput.value);
}

// Enter picks the first suggestion
function handleMaterialSearchKey(event, prefix) {

    if (event.key !== "Enter") {
        return;
    }

    const optionsBox =
        document.getElementById(prefix + "-material-id-options");

    if (!optionsBox || optionsBox.classList.contains("hidden")) {
        return;
    }

    const first = optionsBox.querySelector(
        ".combobox-option:not(.combobox-empty)");

    if (first) {

        event.preventDefault();

        first.dispatchEvent(new MouseEvent("mousedown", { bubbles: true }));
    }
}

function selectMaterialOption(prefix, id) {

    document.getElementById(
        prefix + "-material-id"
    ).value = id;

    // Full "ID - Name" label, same as materialComboboxLabel() used by
    // initMaterialCombobox() - so a Material picked from the dropdown
    // reads the same way whether it was just now selected or loaded
    // in already selected.

    document.getElementById(
        prefix + "-material-id-search"
    ).value = materialComboboxLabel(id);

    document.getElementById(
        prefix + "-material-id-options"
    ).classList.add("hidden");

    updateMaterialPartNumberHint(prefix, id);

    updateMaterialCurrentPriceHint(prefix, id);
}

// Optional: if a form built with materialComboboxHtml(prefix, ...)
// also has an element "<prefix>-material-id-currentprice", this fills
// it in with that material's last recorded price whenever the
// selection changes. Forms without that element are unaffected.

async function updateMaterialCurrentPriceHint(prefix, materialID) {

    const hint =
        document.getElementById(
            prefix + "-material-id-currentprice"
        );

    if (!hint) {
        return;
    }

    if (!materialID) {

        hint.textContent = "";

        return;
    }

    hint.textContent =
        "Loading current price...";

    try {

        const response =
            await fetch(
                `/api/purchase/materials/${encodeURIComponent(materialID)}/history`
            );

        if (!response.ok) {

            hint.textContent = "";

            return;
        }

        const data =
            await response.json();

        const history =
            data.history || [];

        if (history.length === 0) {

            hint.textContent =
                "No previous price on record for this Material.";
        }
        else {

            const latest =
                history[history.length - 1];

            hint.textContent =
                `Previous price: ${formatMoney(data.currentUnitPriceEUR)} EUR ` +
                `(${latest.date})`;
        }
    }
    catch (error) {

        console.error(error);

        hint.textContent = "";
    }
}

function hideMaterialOptionsDelayed(prefix) {

    // Delay so a click (onmousedown) on an option
    // still registers before the dropdown disappears.

    setTimeout(() => {

        const optionsBox =
            document.getElementById(prefix + "-material-id-options");

        if (optionsBox) {
            optionsBox.classList.add("hidden");
        }

    }, 150);
}

// ============================================================
// MATERIAL ID COMBOBOX MARKUP HELPER
// ============================================================
// Returns the HTML for a Material ID search box with the given
// prefix, so every form that needs one builds it the same way.

function materialComboboxHtml(prefix, placeholder) {

    return `
        <div class="combobox">

            <input
                type="text"
                id="${prefix}-material-id-search"
                autocomplete="off"
                placeholder="${placeholder || "Search by ID or name..."}"
                oninput="handleMaterialSearchInput('${prefix}')"
                onkeydown="handleMaterialSearchKey(event, '${prefix}')"
                onfocus="this.select(); renderMaterialOptions('${prefix}')"
                onblur="hideMaterialOptionsDelayed('${prefix}')"
            >

            <input type="hidden" id="${prefix}-material-id">

            <div id="${prefix}-material-id-options"
                class="combobox-options hidden">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE MATERIAL
// ============================================================

async function createMaterial() {

    syncSupplierList("material");

    const id =
        document.getElementById(
            "material-id"
        ).value.trim();

    const name =
        document.getElementById(
            "material-name"
        ).value.trim();

    const description =
        document.getElementById(
            "material-description"
        ).value.trim();

    const uom =
        document.getElementById(
            "material-uom"
        ).value.trim();

    const category =
        document.getElementById(
            "material-category"
        ).value.trim();

    const type =
        document.getElementById(
            "material-type"
        ).value;

    const drawingNumber =
        document.getElementById(
            "material-drawing-number"
        ).value.trim();

    const drawingVersion =
        document.getElementById(
            "material-drawing-version"
        ).value.trim();

    const manufacturer =
        document.getElementById(
            "material-manufacturer"
        ).value.trim();

    const manufacturerPartNumber =
        document.getElementById(
            "material-manufacturer-pn"
        ).value.trim();

    const supplier =
        document.getElementById(
            "material-supplier"
        ).value.trim();

    const supplierPartNumber =
        document.getElementById(
            "material-supplier-pn"
        ).value.trim();

    const active =
        document.getElementById(
            "material-active"
        ).checked;

    const photoInput =
        document.getElementById(
            "material-photo"
        );

    const photoFile =
        photoInput.files[0];

    let photoData = "";
    let photoName = "";

    if (photoFile) {

        photoName =
            photoFile.name;

        photoData =
            await readFileAsBase64(
                photoFile
            );
    }

    // ========================================================
    // VALIDATION
    // ========================================================

    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;

    const drawingNumberPattern =
        /^([0-9]{3}-ASM-[0-9]{4}|[0-9]{3}-PAR-[0-9]{4}|I-BU[0-9]-[0-9]{2}|OWI-BU[0-9]-[0-9]{2}|[0-9]{3}-PCB-[0-9]{4})$/;

    const drawingVersionPattern =
        /^([0-9]+|[A-Z]{2})$/;

    const isDesignOrPCB =
        (type === "Design Part" || type === "PCB");

    const message =
        document.getElementById(
            "material-message"
        );


    if (!idPattern.test(id)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-###### or ######-00.";

        return;
    }


    // Category is optional - a Material without one is grouped under
    // "Other" in the Purchase cost-by-category views.
    if (!name) {

        message.textContent =
            "Please fill in all required fields.";

        return;
    }


    // A Design Part / PCB material is custom-made, so it needs its
    // technical drawing (Drawing Number + Drawing Version) and has
    // no Manufacturer or commercial part number; a Standard Part is
    // the reverse - see toggleDrawingNumberField().

    if (isDesignOrPCB) {

        if (!drawingNumberPattern.test(drawingNumber)) {

            message.textContent =
                "Invalid or missing Drawing Number for this material type.";

            return;
        }

        if (!drawingVersionPattern.test(drawingVersion)) {

            message.textContent =
                "Invalid or missing Drawing Version for this material " +
                "type. Expected a number or two letters (e.g. AA, BA).";

            return;
        }
    }

    // The Supplier itself is optional here for every Material type -
    // a material can be registered before a Supplier has been
    // sourced/confirmed. If the field was left empty this is fine;
    // if a name WAS typed, the backend confirms it is a real
    // Supplier.


    // ========================================================
    // PREPARE JSON
    // ========================================================

    const material = {

        id: id,

        name: name,

        description: description,

        uom: uom || "pcs",

        category: category,

        type: type,

        drawingNumber:
            isDesignOrPCB ? drawingNumber : "",

        drawingVersion:
            isDesignOrPCB ? drawingVersion : "",

        manufacturer:
            isDesignOrPCB ? "" : manufacturer,

        manufacturerPartNumber:
            isDesignOrPCB ? "" : manufacturerPartNumber,

        // A custom-made Material (Design Part / PCB) can still have
        // a Supplier - it's just identified by name, since a
        // custom part has no commercial Supplier Part Number.
        supplier: supplier,

        supplierPartNumber:
            isDesignOrPCB ? "" : supplierPartNumber,

        additionalSuppliers:
            collectAdditionalSuppliers("material").map(link =>
                isDesignOrPCB
                    ? { supplier: link.supplier, supplierPartNumber: "" }
                    : link),

        photo: photoName,

        photoData: photoData,

        active: active
    };


    // ========================================================
    // SEND TO C++
    // ========================================================

    try {

        const response =
            await fetch(
                "/api/materials/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(material)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material created successfully.";


            document.getElementById(
                "material-id"
            ).value = "";

            document.getElementById(
                "material-name"
            ).value = "";

            document.getElementById(
                "material-description"
            ).value = "";

            document.getElementById(
                "material-uom"
            ).value = "pcs";

            document.getElementById(
                "material-category"
            ).value = "";

            document.getElementById(
                "material-type"
            ).value = "Standard Part";

            document.getElementById(
                "material-drawing-number"
            ).value = "";

            document.getElementById(
                "material-drawing-version"
            ).value = "";

            toggleDrawingNumberField("material");

            document.getElementById(
                "material-manufacturer"
            ).value = "";

            document.getElementById(
                "material-manufacturer-pn"
            ).value = "";

            document.getElementById(
                "material-supplier"
            ).value = "";

            document.getElementById(
                "material-supplier-search"
            ).value = "";

            document.getElementById(
                "material-supplier-pn"
            ).value = "";

            // Supplier list back to one empty (primary) row
            document.getElementById("material-addsup-lines").innerHTML = "";

            additionalSupplierCounters["material"] = 0;

            addAdditionalSupplierRow("material", "", "", true);

            document.getElementById(
                "material-photo"
            ).value = "";

            document.getElementById(
                "material-active"
            ).checked = true;

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY MATERIALS
// ============================================================

async function displayMaterials() {

    await renderMaterialsList(
        material => material.active !== false,
        "Materials",
        "No active materials available. Check Archived Materials " +
        "for materials marked inactive."
    );
}

// ============================================================
// ARCHIVED MATERIALS (inactive materials only)
// ============================================================
// Materials are never hard-deleted while still referenced anywhere
// in the system (Delete Material refuses that) - marking a Material
// inactive via its "Active Material" checkbox is how a discontinued
// or no-longer-purchased material is taken out of the everyday
// Display Materials list without losing its history.

async function displayArchivedMaterials() {

    await renderMaterialsList(
        material => material.active === false,
        "Archived Materials",
        "No archived (inactive) materials."
    );
}

// ============================================================
// MATERIALS TABLE - SHARED RENDERING (active or archived)
// ============================================================

async function renderMaterialsList(filterFn, headingText, emptyText, targetID) {

    lastMaterialsList = {
        filterFn: filterFn,
        headingText: headingText,
        emptyText: emptyText,
        targetID: targetID
    };

    const content =
        document.getElementById(
            targetID || "material-content"
        );

    content.innerHTML = `
        <p>Loading materials...</p>
    `;


    try {

        const response =
            await fetch(
                "/api/materials"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();

        const listed =
            sortMaterialsByID(
                (data.materials || []).filter(filterFn));

        if (listed.length === 0) {

            content.innerHTML = `

                <div class="empty-message">

                    ${escapeHtml(emptyText)}

                </div>
            `;

            return;
        }

        // Supplier filter: primary or additional supplier
        const materialSuppliers = material =>
            [material.supplier].concat(
                (material.additionalSuppliers || [])
                    .map(link => link.supplier))
                .filter(name => name);

        const supplierNames = Array.from(new Set(
            listed.flatMap(materialSuppliers)))
            .sort((a, b) => a.localeCompare(b));

        if (materialsSupplierFilter &&
            !supplierNames.includes(materialsSupplierFilter) &&
            materialsSupplierFilter !== "__none__") {

            materialsSupplierFilter = "";
        }

        const materials = listed.filter(material => {

            if (!materialsSupplierFilter) {
                return true;
            }

            if (materialsSupplierFilter === "__none__") {
                return materialSuppliers(material).length === 0;
            }

            return materialSuppliers(material)
                .includes(materialsSupplierFilter);
        });

        const filterHtml = `
            <div style="margin:10px 0;">
                <label for="materials-supplier-filter"
                    style="display:inline; margin-right:8px;">
                    <strong>Supplier</strong>
                </label>
                <select id="materials-supplier-filter"
                    style="max-width:320px;"
                    onchange="setMaterialsSupplierFilter(this.value)">
                    <option value="">All suppliers (${listed.length})</option>
                    ${supplierNames.map(name => `
                        <option value="${escapeHtml(name)}"
                            ${name === materialsSupplierFilter ? "selected" : ""}>
                            ${escapeHtml(name)}
                        </option>`).join("")}
                    <option value="__none__"
                        ${materialsSupplierFilter === "__none__" ? "selected" : ""}>
                        No supplier
                    </option>
                </select>
                ${materialsSupplierFilter ?
                    `<span style="margin-left:10px; color:#6b7280;">
                        ${materials.length} material(s)</span>` : ""}
            </div>
        `;

        if (materials.length === 0) {

            content.innerHTML = `
                <h2>${escapeHtml(headingText)}</h2>
                ${filterHtml}
                <div class="empty-message">
                    No materials for this supplier.
                </div>
            `;

            return;
        }


        let html = `

            <h2>
                ${escapeHtml(headingText)}
            </h2>

            ${filterHtml}

            <div class="material-table-container material-table-container-fit">

                <table class="material-table material-table-clickable">

                    <colgroup>
                        <col style="width: 6%">
                        <col style="width: 9%">
                        <col style="width: 14%">
                        <col style="width: 5%">
                        <col style="width: 11%">
                        <col style="width: 6%">
                        <col style="width: 12%">
                        <col style="width: 11%">
                        <col style="width: 15%">
                        <col style="width: 11%">
                    </colgroup>

                    <thead>

                        <tr>

                            <th>Photo</th>
                            <th>Material ID</th>
                            <th class="material-name-cell">Name</th>
                            <th>UoM</th>
                            <th>Drawing Number</th>
                            <th class="material-drawing-version-cell">Drawing<br>Version</th>
                            <th>Manufacturer</th>
                            <th>Mfr Part #</th>
                            <th>Supplier</th>
                            <th>Supplier Part #</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (
            const material of materials
        ) {

            html += `

                <tr>

                    <td>

                        ${
                            material.photo
                            ? `<img
                                    src="/${material.photo}"
                                    class="material-thumbnail"
                                    alt="Material photo"
                               >`
                            : "No photo"
                        }

                    </td>

                    <td>
                        <a
                            href="#"
                            class="material-id-link"
                            title="View full material details"
                            onclick="event.preventDefault(); openMaterialFromList('${escapeHtml(material.id)}');">

                            ${escapeHtml(material.id)}

                        </a>
                    </td>

                    <td class="material-name-cell">
                        ${escapeHtml(material.name)}
                        ${material.active === false ?
                            `<span class="status-badge status-inactive">Archived</span>` : ""}
                    </td>

                    <td>
                        ${escapeHtml(material.uom || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.drawingNumber || "")}
                    </td>

                    <td class="material-drawing-version-cell">
                        ${escapeHtml(material.drawingVersion || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.manufacturer || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.manufacturerPartNumber || "")}
                    </td>

                    <td>
                        ${escapeHtml(material.supplier || "")}
                        ${
                            (material.additionalSuppliers || []).length > 0
                            ? ` <span class="status-badge" title="${
                                    escapeHtml(material.additionalSuppliers
                                        .map(link => link.supplier)
                                        .join(", "))
                                }">+${material.additionalSuppliers.length}</span>`
                            : ""
                        }
                    </td>

                    <td>
                        ${escapeHtml(material.supplierPartNumber || "")}
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

        enableColumnFilters(content);

        const tableContainer =
            content.querySelector(".material-table-container");

        if (tableContainer) {
            tableContainer.scrollLeft = 0;
        }

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

function setMaterialsSupplierFilter(value) {

    materialsSupplierFilter = value;

    if (lastMaterialsList) {

        renderMaterialsList(
            lastMaterialsList.filterFn,
            lastMaterialsList.headingText,
            lastMaterialsList.emptyText,
            lastMaterialsList.targetID);
    }
}

// ============================================================
// MATERIAL DETAIL DASHBOARD
// ============================================================

async function showMaterialDetail(id) {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `
        <p>Loading material...</p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `
                <p>
                    ${escapeHtml(data.message || "Material not found.")}
                </p>

                <div class="module-buttons">
                    <button onclick="displayMaterials()">
                        ← Back to Materials
                    </button>
                </div>
            `;

            return;
        }


        content.innerHTML = `

            <div class="material-detail">

                <div class="module-buttons material-detail-back">
                    <button onclick="goBackFromMaterialDetail()">
                        ← Back
                    </button>

                    <button onclick="modifyMaterialFromDetail('${escapeHtml(data.id)}')">
                        Modify Material
                    </button>
                </div>

                <div class="material-detail-header">

                    <div class="material-detail-photo">
                        ${
                            data.photo
                            ? `<img
                                    src="/${data.photo}"
                                    alt="Material photo"
                               >`
                            : `<div class="material-detail-photo-placeholder">
                                    No photo
                               </div>`
                        }
                    </div>

                    <div class="material-detail-title">

                        <h2>
                            ${escapeHtml(data.name)}
                        </h2>

                        <p class="material-detail-id">
                            ${escapeHtml(data.id)}
                        </p>

                        <span class="status-badge ${data.active ? "status-active" : "status-inactive"}">
                            ${data.active ? "Active" : "Inactive"}
                        </span>

                    </div>

                </div>

                <div class="material-detail-grid">

                    <div class="detail-field">
                        <div class="detail-label">Description</div>
                        <div class="detail-value">${escapeHtml(data.description || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Unit of Measure</div>
                        <div class="detail-value">${escapeHtml(data.uom || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Category</div>
                        <div class="detail-value">${escapeHtml(data.category || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Type</div>
                        <div class="detail-value">${escapeHtml(data.type || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Drawing Number</div>
                        <div class="detail-value">${escapeHtml(data.drawingNumber || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Drawing Version</div>
                        <div class="detail-value">${escapeHtml(data.drawingVersion || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Manufacturer</div>
                        <div class="detail-value">${escapeHtml(data.manufacturer || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Manufacturer Part Number</div>
                        <div class="detail-value">${escapeHtml(data.manufacturerPartNumber || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Primary Supplier</div>
                        <div class="detail-value">${escapeHtml(data.supplier || "—")}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Primary Supplier Part Number</div>
                        <div class="detail-value">${escapeHtml(data.supplierPartNumber || "—")}</div>
                    </div>

                </div>

                ${
                    (data.additionalSuppliers || []).length > 0
                    ? `
                        <div class="procurement-section">

                            <h3>
                                Additional Suppliers
                            </h3>

                            <table class="material-table">

                                <thead>
                                    <tr>
                                        <th>Supplier</th>
                                        <th>Supplier Part Number</th>
                                    </tr>
                                </thead>

                                <tbody>
                                    ${data.additionalSuppliers.map(link => `
                                        <tr>
                                            <td>${escapeHtml(link.supplier)}</td>
                                            <td>${escapeHtml(link.supplierPartNumber || "—")}</td>
                                        </tr>
                                    `).join("")}
                                </tbody>

                            </table>

                        </div>
                    `
                    : ""
                }

                <div id="material-movements"></div>

            </div>
        `;

        loadMaterialMovements(data.id);
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// SEARCH MATERIAL FORM
// ============================================================

function showSearchMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Search Material
            </h2>

            <label>
                Search
            </label>

            <input
                type="text"
                id="search-material-query"
                placeholder="ID, name, description, supplier, part number, drawing..."
                autocomplete="off"
                onkeydown="if (event.key === 'Enter') searchMaterial();"
            >

            <small>
                Finds every material (active or archived) where all the
                words typed appear in any of: ID, name, description,
                category, drawing number, manufacturer, part numbers or
                suppliers.
            </small>

            <div class="form-actions">

                <button
                    onclick="searchMaterial()">

                    Search Material

                </button>

            </div>

        </div>

        <div id="search-material-result">
        </div>
    `;

    document.getElementById("search-material-query").focus();
}


// ============================================================
// SEARCH MATERIAL
// ============================================================
// Any coincidence: every word typed must appear somewhere in the
// material's fields. Results use the same table as Display Materials
// (photo, clickable ID, supplier filter).

function materialSearchText(material) {

    return [
        material.id, material.name, material.description,
        material.uom, material.category, material.type,
        material.drawingNumber, material.manufacturer,
        material.manufacturerPartNumber, material.supplier,
        material.supplierPartNumber
    ].concat(
        (material.additionalSuppliers || []).map(
            link => (link.supplier || "") + " " +
                (link.supplierPartNumber || ""))
    ).join(" ").toLowerCase();
}

async function searchMaterial() {

    const query =
        document.getElementById(
            "search-material-query"
        ).value.trim();

    const result =
        document.getElementById(
            "search-material-result"
        );

    if (!query) {

        result.innerHTML = `
            <p>Type something to search.</p>
        `;

        return;
    }

    const words = query.toLowerCase().split(/\s+/).filter(w => w);

    materialsSupplierFilter = "";

    await renderMaterialsList(
        material => {

            const text = materialSearchText(material);

            return words.every(word => text.includes(word));
        },
        `Results for "${query}"`,
        `No materials match "${query}".`,
        "search-material-result"
    );
}


// ============================================================
// MODIFY MATERIAL FORM
// ============================================================

// Jumps straight from the Material Detail dashboard into the Modify
// Material form, with that same material already selected and
// loaded - so a change from there doesn't need to be looked up again
// by ID.

async function modifyMaterialFromDetail(id) {

    materialModifyBackID = id;

    showModifyMaterial();

    await initMaterialCombobox("modify", id);

    loadMaterialForModify();
}

// Back from the Modify Material form to the Material detail it was
// opened from (re-fetched, so it shows any change just saved; its own
// Back button keeps pointing at the list / product / projection).
function goBackFromModifyMaterial() {

    const id = materialModifyBackID;

    materialModifyBackID = null;

    if (id) {
        showMaterialDetail(id);
    }
    else {
        displayMaterials();
    }
}

function showModifyMaterial(statusMessage) {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            ${materialModifyBackID ? `
                <div class="module-buttons">
                    <button
                        class="button-secondary"
                        onclick="goBackFromModifyMaterial()">

                        ← Back

                    </button>
                </div>
            ` : ""}

            <h2>
                Modify Material
            </h2>

            ${statusMessage ?
                `<p class="success-message">${escapeHtml(statusMessage)}</p>` :
                ""}

            <label>
                Material ID
            </label>

            ${materialComboboxHtml("modify")}

            <div class="form-actions">

                <button
                    onclick="loadMaterialForModify()">

                    Load Material

                </button>

            </div>

            <div id="modify-form">
            </div>

        </div>
    `;

    initMaterialCombobox("modify");
}

// ============================================================
// DELETE MATERIAL FORM
// ============================================================

function showDeleteMaterial() {

    const content =
        document.getElementById(
            "material-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Material
            </h2>


            <label>
                Material ID
            </label>

            ${materialComboboxHtml("delete")}


            <div class="form-actions">

                <button
                    onclick="loadMaterialForDelete()">

                    Find Material

                </button>

            </div>


            <div id="delete-content">
            </div>

        </div>
    `;

    initMaterialCombobox("delete");
}

// ============================================================
// LOAD MATERIAL FOR DELETE
// ============================================================

async function loadMaterialForDelete() {

    const id =
        document.getElementById(
            "delete-material-id"
        ).value.trim();


    const content =
        document.getElementById(
            "delete-content"
        );


    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!idPattern.test(id)) {

        content.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-###### or ######-00.
            </p>
        `;

        return;
    }


    content.innerHTML = `
        <p>
            Loading material...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        content.innerHTML = `

            <div class="search-result">

                <h3>
                    Material to Delete
                </h3>

                <p>
                    <strong>ID:</strong>
                    ${data.id}
                </p>

                <p>
                    <strong>Name:</strong>
                    ${escapeHtml(data.name)}
                </p>

                <p>
                    <strong>Description:</strong>
                    ${escapeHtml(data.description || "")}
                </p>

                <p>
                    <strong>Category:</strong>
                    ${escapeHtml(data.category)}
                </p>

                <p>
                    <strong>Supplier:</strong>
                    ${escapeHtml(data.supplier)}
                </p>


                <div class="delete-confirmation">

                    <p>
                        <strong>
                            Are you sure you want to delete
                            this material?
                        </strong>
                    </p>


                    <button
                        class="delete-button"
                        onclick="deleteMaterial(
                            '${data.id}'
                        )">

                        Yes, Delete Material

                    </button>


                    <button
                        class="cancel-button"
                        onclick="showDeleteMaterial()">

                        Cancel

                    </button>

                </div>

            </div>

        `;
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE MATERIAL
// ============================================================

async function deleteMaterial(id) {

    const content =
        document.getElementById(
            "material-content"
        );


    try {

        const response =
            await fetch(
                "/api/materials/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: id
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            content.innerHTML = `

                <div class="search-result">

                    <h3>
                        Material Deleted
                    </h3>

                    <p>
                        Material
                        <strong>${escapeHtml(id)}</strong>
                        was deleted successfully.
                    </p>

                </div>
            `;
        }
        else {

            content.innerHTML = `

                <div class="search-result">

                    <p>
                        Error: ${escapeHtml(responseText)}
                    </p>

                </div>
            `;
        }
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// LOAD MATERIAL FOR MODIFY
// ============================================================

async function loadMaterialForModify() {

    const id =
        document.getElementById(
            "modify-material-id"
        ).value.trim();


    const modifyForm =
        document.getElementById(
            "modify-form"
        );


    const idPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    if (!idPattern.test(id)) {

        modifyForm.innerHTML = `
            <p>
                Invalid Material ID.
                Expected format ###-###### or ######-00.
            </p>
        `;

        return;
    }


    modifyForm.innerHTML = `
        <p>
            Loading material...
        </p>
    `;


    try {

        const response =
            await fetch(
                `/api/materials/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            modifyForm.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }


        modifyForm.innerHTML = `

            <div class="form-container">

                <div style="display:flex; justify-content:space-between; align-items:flex-start; gap:16px;">

                    <div>
                        <h2>
                            Modify Material
                        </h2>

                        <p>
                            Material ID:
                            <strong>${data.id}</strong>
                        </p>
                    </div>

                    <img
                        id="modify-photo-thumb"
                        src="${data.photo ? "/" + escapeHtml(data.photo) : ""}"
                        alt="Material photo"
                        style="width:96px; height:96px; object-fit:cover; border-radius:8px; border:1px solid #d1d5db; background:#fff; flex-shrink:0; ${data.photo ? "" : "display:none;"}"
                    >

                </div>


                <label>
                    Material Name
                </label>

                <input
                    type="text"
                    id="modify-name"
                    value="${escapeHtml(data.name)}"
                >


                <label>
                    Description
                </label>

                <textarea
                    id="modify-description"
                    rows="4"
                >${escapeHtml(data.description || "")}</textarea>


                <label>
                    Unit of Measure
                </label>

                <input
                    type="text"
                    id="modify-uom"
                    value="${escapeHtml(data.uom)}"
                >


                <label>
                    Category (optional)
                </label>

                <input
                    type="text"
                    id="modify-category"
                    value="${escapeHtml(data.category)}"
                >


                <label>
                    Material Type
                </label>

                <select
                    id="modify-type"
                    onchange="toggleDrawingNumberField('modify')">

                    <option value="Standard Part"
                        ${data.type === "Standard Part" ? "selected" : ""}>
                        Standard Part
                    </option>

                    <option value="Design Part"
                        ${data.type === "Design Part" ? "selected" : ""}>
                        Design Part
                    </option>

                    <option value="PCB"
                        ${data.type === "PCB" ? "selected" : ""}>
                        PCB
                    </option>

                </select>


                <div id="modify-drawing-number-group"
                    class="${
                        (data.type === "Design Part" || data.type === "PCB")
                        ? ""
                        : "hidden"
                    }">

                    <label>
                        Drawing Number
                    </label>

                    <input
                        type="text"
                        id="modify-drawing-number"
                        value="${escapeHtml(data.drawingNumber || "")}"
                        placeholder="e.g. 123-ASM-4567"
                    >

                    <small>
                        Format: ###-ASM-####, ###-PAR-####,
                        I-BU#-##, OWI-BU#-##, or ###-PCB-####
                    </small>

                    <label>
                        Drawing Version
                    </label>

                    <input
                        type="text"
                        id="modify-drawing-version"
                        value="${escapeHtml(data.drawingVersion || "")}"
                        placeholder="e.g. AA, BA, or 1"
                    >

                    <small>
                        A number (1, 2, ...) or two letters (AA, BA, ...)
                    </small>

                </div>


                <div id="modify-manufacturer-fields-group"
                    class="${
                        (data.type === "Design Part" || data.type === "PCB")
                        ? "hidden"
                        : ""
                    }">

                    <label>
                        Manufacturer
                    </label>

                    <input
                        type="text"
                        id="modify-manufacturer"
                        value="${escapeHtml(data.manufacturer || "")}"
                    >


                    <label>
                        Manufacturer Part Number
                    </label>

                    <input
                        type="text"
                        id="modify-manufacturer-pn"
                        value="${escapeHtml(data.manufacturerPartNumber || "")}"
                    >

                </div>


                <div id="modify-supplier-fields-group">

                    <!-- Kept (hidden) for the rest of the code: the
                         supplier marked "Primary" in the list below is
                         copied into these fields on save. -->
                    <div style="display: none;">

                    <label>
                        Primary Supplier
                    </label>

                    <div class="combobox">

                        <input
                            type="text"
                            id="modify-supplier-search"
                            autocomplete="off"
                            placeholder="Search supplier by name..."
                            oninput="handleSupplierSearchInput('modify')"
                            onfocus="renderSupplierOptions('modify')"
                            onblur="hideSupplierOptionsDelayed('modify')"
                        >

                        <input type="hidden" id="modify-supplier">

                        <div id="modify-supplier-options"
                            class="combobox-options hidden">
                        </div>

                    </div>


                    <div id="modify-supplier-pn-group"
                        class="${
                            (data.type === "Design Part" || data.type === "PCB")
                            ? "hidden"
                            : ""
                        }">

                        <label>
                            Primary Supplier Part Number
                        </label>

                        <input
                            type="text"
                            id="modify-supplier-pn"
                            value="${escapeHtml(data.supplierPartNumber || "")}"
                        >

                    </div>

                    </div>

                    <label>
                        Suppliers
                    </label>

                    <small>
                        Add every supplier this Material can be purchased
                        from and tick <strong>Primary</strong> on the main
                        one. To change the primary supplier later, just
                        tick another row.
                    </small>

                    <div id="modify-addsup-lines">
                    </div>

                    <button
                        type="button"
                        onclick="addAdditionalSupplierRow('modify')">

                        + Add Supplier

                    </button>

                </div>

                <label>
                    Material Photo
                </label>

                <input
                    type="file"
                    id="modify-photo-input"
                    accept="image/*"
                >

                <input
                    type="hidden"
                    id="modify-photo"
                    value="${escapeHtml(data.photo || "")}"
                >

                <label class="checkbox-label">

                    <input
                        type="checkbox"
                        id="modify-active"
                        ${data.active ? "checked" : ""}
                    >

                    Active Material

                </label>


                <div class="form-actions">

                    <button
                        onclick="modifyMaterial()">

                        Save Changes

                    </button>

                </div>


                <div id="modify-message">
                </div>

            </div>

        `;

        initSupplierCombobox(
            "modify",
            data.supplier);

        // the thumbnail follows a newly chosen photo (before saving)
        const modifyPhotoInput =
            document.getElementById("modify-photo-input");

        if (modifyPhotoInput) {

            modifyPhotoInput.addEventListener("change", () => {

                const file = modifyPhotoInput.files[0];

                const thumb = document.getElementById("modify-photo-thumb");

                if (file && thumb) {

                    thumb.src = URL.createObjectURL(file);

                    thumb.style.display = "block";
                }
            });
        }

        additionalSupplierCounters["modify"] = 0;

        // The list starts with the current primary supplier (marked),
        // followed by the additional ones
        if (data.supplier) {

            addAdditionalSupplierRow(
                "modify",
                data.supplier,
                data.supplierPartNumber,
                true
            );
        }

        for (const link of (data.additionalSuppliers || [])) {

            addAdditionalSupplierRow(
                "modify",
                link.supplier,
                link.supplierPartNumber
            );
        }

        // Nothing to list yet: one empty row, so the form still shows
        // the supplier list
        if (!data.supplier &&
            (data.additionalSuppliers || []).length === 0) {

            addAdditionalSupplierRow("modify", "", "", true);
        }
    }
    catch (error) {

        console.error(error);

        modifyForm.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// HTML ESCAPE
// ============================================================

// ================================================================
// QUANTITY HELPERS (material quantities allow up to 3 decimals)
// ================================================================

// Round to 3 decimals (avoids 0.1 + 0.2 = 0.30000000000000004)
function roundQty3(value) {
    const number = Number(value);

    if (!Number.isFinite(number)) {
        return 0;
    }

    return Math.round(number * 1000) / 1000;
}

// True when the number has no more than 3 decimals
function hasMaxThreeDecimals(value) {
    const number = Number(value);

    if (!Number.isFinite(number)) {
        return false;
    }

    return Math.abs(number * 1000 - Math.round(number * 1000)) < 1e-6;
}

function escapeHtml(value) {

    return String(value)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

// ============================================================
// CONFIRM DIALOG (in-page, instead of the browser's own
// window.confirm popup - built once and reused for every call)
// ============================================================

function showConfirmDialog(message) {

    return new Promise((resolve) => {

        let overlay =
            document.getElementById(
                "app-confirm-overlay"
            );

        if (!overlay) {

            overlay =
                document.createElement("div");

            overlay.id =
                "app-confirm-overlay";

            overlay.className =
                "app-modal-overlay hidden";

            overlay.innerHTML = `
                <div class="app-modal-box">

                    <p id="app-confirm-message">
                    </p>

                    <div class="app-modal-actions">

                        <button
                            type="button"
                            id="app-confirm-cancel"
                            class="app-modal-button-secondary">

                            Cancel

                        </button>

                        <button
                            type="button"
                            id="app-confirm-ok"
                            class="app-modal-button-primary">

                            OK

                        </button>

                    </div>

                </div>
            `;

            document.body.appendChild(overlay);
        }

        document.getElementById(
            "app-confirm-message"
        ).textContent = message;

        overlay.classList.remove("hidden");

        const okButton =
            document.getElementById("app-confirm-ok");

        const cancelButton =
            document.getElementById("app-confirm-cancel");

        function finish(result) {

            overlay.classList.add("hidden");

            okButton.onclick = null;
            cancelButton.onclick = null;

            resolve(result);
        }

        okButton.onclick = () => finish(true);
        cancelButton.onclick = () => finish(false);
    });
}

// ============================================================
// MODIFY MATERIAL
// ============================================================

async function modifyMaterial() {

    syncSupplierList("modify");

    const id =
        document.getElementById(
            "modify-material-id"
        ).value.trim();

    const name =
        document.getElementById(
            "modify-name"
        ).value.trim();

    const description =
        document.getElementById(
            "modify-description"
        ).value.trim();

    const uom =
        document.getElementById(
            "modify-uom"
        ).value.trim();

    const category =
        document.getElementById(
            "modify-category"
        ).value.trim();

    const type =
        document.getElementById(
            "modify-type"
        ).value;

    const drawingNumber =
        document.getElementById(
            "modify-drawing-number"
        ).value.trim();

    const drawingVersion =
        document.getElementById(
            "modify-drawing-version"
        ).value.trim();

    const manufacturer =
        document.getElementById(
            "modify-manufacturer"
        ).value.trim();

    const manufacturerPartNumber =
        document.getElementById(
            "modify-manufacturer-pn"
        ).value.trim();

    const supplier =
        document.getElementById(
            "modify-supplier"
        ).value.trim();

    const supplierPartNumber =
        document.getElementById(
            "modify-supplier-pn"
        ).value.trim();

    const active =
        document.getElementById(
            "modify-active"
        ).checked;

    const photoInput =
        document.getElementById(
            "modify-photo-input"
        );

    const photoFile =
        photoInput.files[0];

    let photoPath =
        document.getElementById(
            "modify-photo"
        ).value;


    let photoData = "";


    if (photoFile)
    {
        photoData =
            await readFileAsBase64(
                photoFile
            );
    }

    const message =
        document.getElementById(
            "modify-message"
        );

    const drawingNumberPattern =
        /^([0-9]{3}-ASM-[0-9]{4}|[0-9]{3}-PAR-[0-9]{4}|I-BU[0-9]-[0-9]{2}|OWI-BU[0-9]-[0-9]{2}|[0-9]{3}-PCB-[0-9]{4})$/;

    const drawingVersionPattern =
        /^([0-9]+|[A-Z]{2})$/;

    const isDesignOrPCB =
        (type === "Design Part" || type === "PCB");

    // Category is optional - a Material without one is grouped under
    // "Other" in the Purchase cost-by-category views.
    if (!name) {

        message.textContent =
            "Please fill in all required fields.";

        return;
    }


    // A Design Part / PCB material is custom-made, so it needs its
    // technical drawing (Drawing Number + Drawing Version) and has
    // no Manufacturer or commercial part number; a Standard Part is
    // the reverse - see toggleDrawingNumberField().

    if (isDesignOrPCB) {

        if (!drawingNumberPattern.test(drawingNumber)) {

            message.textContent =
                "Invalid or missing Drawing Number for this material type.";

            return;
        }

        if (!drawingVersionPattern.test(drawingVersion)) {

            message.textContent =
                "Invalid or missing Drawing Version for this material " +
                "type. Expected a number or two letters (e.g. AA, BA).";

            return;
        }
    }

    // Supplier is optional here too, for every Material type - see
    // the same note in createMaterial().


    const material = {

        id: id,

        name: name,

        description: description,

        uom: uom || "pcs",

        category: category,

        type: type,

        drawingNumber:
            isDesignOrPCB ? drawingNumber : "",

        drawingVersion:
            isDesignOrPCB ? drawingVersion : "",

        manufacturer:
            isDesignOrPCB ? "" : manufacturer,

        manufacturerPartNumber:
            isDesignOrPCB ? "" : manufacturerPartNumber,

        // A custom-made Material (Design Part / PCB) can still have
        // a Supplier - see the same note in createMaterial().
        supplier: supplier,

        supplierPartNumber:
            isDesignOrPCB ? "" : supplierPartNumber,

        additionalSuppliers:
            collectAdditionalSuppliers("modify").map(link =>
                isDesignOrPCB
                    ? { supplier: link.supplier, supplierPartNumber: "" }
                    : link),

        photo: photoPath,

        photoData: photoData,

        active: active
    };


    try {

        const response =
            await fetch(
                "/api/materials/modify",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(material)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            // Re-render the form empty (combobox reset, no fields
            // filled in) so it is visually obvious the change was
            // saved, rather than leaving the just-saved values on
            // screen looking unchanged.
            showModifyMaterial(
                "Material successfully modified.");

            return;
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// CREATE WAREHOUSE FORM
// ============================================================

function showCreateWarehouse() {

    const content =
        document.getElementById(
            "warehouse-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Warehouse
            </h2>


            <label>
                Warehouse ID
            </label>

            <input
                type="number"
                id="warehouse-id"
                placeholder="Enter Warehouse ID"
            >


            <label>
                Warehouse Name
            </label>

            <input
                type="text"
                id="warehouse-name"
                placeholder="Enter Warehouse Name"
            >


            <div class="form-actions">

                <button
                    onclick="createWarehouse()">

                    Create Warehouse

                </button>

            </div>


            <div id="warehouse-message">
            </div>

        </div>
    `;
}

// ============================================================
// CREATE WAREHOUSE
// ============================================================

async function createWarehouse() {

    const id =
        document.getElementById(
            "warehouse-id"
        ).value;

    const name =
        document.getElementById(
            "warehouse-name"
        ).value.trim();


    const message =
        document.getElementById(
            "warehouse-message"
        );


    if (!id || !name) {

        message.textContent =
            "Please fill in all required fields.";

        return;
    }


    const warehouse = {

        id: Number(id),

        name: name

    };


    try {

        const response =
            await fetch(
                "/api/warehouses/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(warehouse)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Warehouse created successfully.";


            document.getElementById(
                "warehouse-id"
            ).value = "";

            document.getElementById(
                "warehouse-name"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY WAREHOUSES
// ============================================================

async function displayWarehouses() {

    const content =
        document.getElementById(
            "warehouse-content"
        );


    content.innerHTML = `
        <p>
            Loading warehouses...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/warehouses"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.warehouses ||
            data.warehouses.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">
                    No warehouses available.
                </div>
            `;

            return;
        }


        let html = `

            <div class="material-table-container">

                <h2>
                    Warehouses
                </h2>

                <table class="material-table">

                    <thead>

                        <tr>

                            <th>Warehouse ID</th>
                            <th>Warehouse Name</th>

                        </tr>

                    </thead>

                    <tbody>
        `;


        for (
            const warehouse of data.warehouses
        ) {

            html += `

                <tr>

                    <td>
                        ${warehouse.id}
                    </td>

                    <td>
                        ${escapeHtml(
                            warehouse.name
                        )}
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

            </div>
        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// DELETE WAREHOUSE FORM
// ============================================================

function showDeleteWarehouse() {

    const content =
        document.getElementById(
            "warehouse-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Warehouse
            </h2>


            <label>
                Warehouse ID
            </label>

            <select id="delete-warehouse-id"></select>


            <div class="form-actions">

                <button
                    onclick="deleteWarehouse()">

                    Delete Warehouse

                </button>

            </div>


            <div id="warehouse-delete-message">
            </div>

        </div>
    `;

    populateWarehouseSelect("delete-warehouse-id", "", WH_OPTS);
}

// ============================================================
// DELETE WAREHOUSE
// ============================================================

async function deleteWarehouse() {

    const id =
        document.getElementById(
            "delete-warehouse-id"
        ).value;


    const message =
        document.getElementById(
            "warehouse-delete-message"
        );


    if (!id) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/warehouses/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: Number(id)
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Warehouse deleted successfully.";


            // Reload the list: the deleted Warehouse is gone from it
            populateWarehouseSelect("delete-warehouse-id", "", WH_OPTS);

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// GOODS RECEIPT FORM
// ============================================================

function showGoodsReceipt() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                Goods Receipt
            </h2>


            <label>
                Warehouse ID
            </label>

            <select id="receipt-warehouse-id"></select>


            <div class="multi-line-items">

                <div class="multi-line-header">

                    <label>
                        Materials
                    </label>

                    <button
                        type="button"
                        class="add-line-button"
                        onclick="addGoodsLine('receipt')">

                        + Add Material

                    </button>

                </div>

                <div id="receipt-lines">
                </div>

            </div>


            <label>
                Comment
            </label>

            <textarea
                id="receipt-comment"
                rows="4"
                placeholder="General comment for this receipt (applies to all materials above)"></textarea>


            <div class="form-actions">

                <button
                    onclick="goodsReceipt()">

                    Receive Goods

                </button>

            </div>


            <div id="receipt-message">
            </div>

        </div>
    `;

    initGoodsLines("receipt");

    populateWarehouseSelect("receipt-warehouse-id", "", WH_OPTS);
}

// ============================================================
// GOODS RECEIPT
// ============================================================

async function goodsReceipt() {

    const warehouseID =
        document.getElementById(
            "receipt-warehouse-id"
        ).value;

    const comment =
        document.getElementById(
            "receipt-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "receipt-message"
        );


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    const lines =
        collectGoodsLines("receipt", message);

    if (!lines) {
        return;
    }


    // --------------------------------------------------------
    // Send each line to C++ (same endpoint, one call per
    // material, sharing the same warehouse and comment)
    // --------------------------------------------------------

    message.textContent =
        "Processing...";

    const results =
        await submitGoodsLines(
            "/api/inventory/receipt",
            warehouseID,
            comment,
            lines
        );

    reportGoodsLinesResult(
        message,
        results,
        "Goods receipt"
    );

    if (results.every(result => result.ok)) {

        document.getElementById(
            "receipt-warehouse-id"
        ).value = "";

        document.getElementById(
            "receipt-comment"
        ).value = "";

        initGoodsLines("receipt");
    }
}

// ============================================================
// GOODS ISSUE FORM
// ============================================================

function showGoodsIssue() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                Goods Issue
            </h2>


            <label>
                Warehouse ID
            </label>

            <select id="issue-warehouse-id"></select>


            <div class="multi-line-items">

                <div class="multi-line-header">

                    <label>
                        Materials
                    </label>

                    <button
                        type="button"
                        class="add-line-button"
                        onclick="addGoodsLine('issue')">

                        + Add Material

                    </button>

                </div>

                <div id="issue-lines">
                </div>

            </div>


            <label>
                Comment
            </label>

            <textarea
                id="issue-comment"
                rows="4"
                placeholder="General comment for this issue (applies to all materials above)"></textarea>


            <div class="form-actions">

                <button
                    onclick="goodsIssue()">

                    Issue Goods

                </button>

            </div>


            <div id="issue-message">
            </div>

        </div>
    `;

    initGoodsLines("issue");

    populateWarehouseSelect("issue-warehouse-id", "", WH_OPTS);
}

// ============================================================
// GOODS ISSUE
// ============================================================

async function goodsIssue() {

    const warehouseID =
        document.getElementById(
            "issue-warehouse-id"
        ).value;

    const comment =
        document.getElementById(
            "issue-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "issue-message"
        );


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    const lines =
        collectGoodsLines("issue", message);

    if (!lines) {
        return;
    }


    // --------------------------------------------------------
    // Send each line to C++ (same endpoint, one call per
    // material, sharing the same warehouse and comment)
    // --------------------------------------------------------

    message.textContent =
        "Processing...";

    const results =
        await submitGoodsLines(
            "/api/inventory/issue",
            warehouseID,
            comment,
            lines
        );

    reportGoodsLinesResult(
        message,
        results,
        "Goods issue"
    );

    if (results.every(result => result.ok)) {

        document.getElementById(
            "issue-warehouse-id"
        ).value = "";

        document.getElementById(
            "issue-comment"
        ).value = "";

        initGoodsLines("issue");
    }
}

// ============================================================
// GOODS RECEIPT / ISSUE - MULTIPLE MATERIAL LINES
// ============================================================
// Both forms let the user add several materials at once under
// one Warehouse ID and one general Comment. Each line keeps its
// own Material ID (via the shared combobox) and Quantity. The
// backend still only knows how to receive/issue one material at
// a time, so each line is sent as its own request to the same
// endpoint, reusing the shared warehouse and comment.

let goodsLineCounters = {
    receipt: 0,
    issue: 0
};

function goodsLineRowHtml(type, rowIndex) {

    const prefix =
        `${type}-line-${rowIndex}`;

    return `
        <div class="line-item" id="${prefix}-row">

            <div class="line-item-field">

                <label>
                    Material ID
                </label>

                ${materialComboboxHtml(prefix)}

            </div>

            <div class="line-item-field line-item-field-qty">

                <label>
                    Quantity
                </label>

                <input
                    type="number"
                    id="${prefix}-quantity"
                    min="0.001"
                    step="any"
                    placeholder="Qty"
                >

            </div>

            <button
                type="button"
                class="remove-line-button"
                title="Remove this material"
                onclick="removeGoodsLine('${type}', ${rowIndex})">

                &times;

            </button>

        </div>
    `;
}

function initGoodsLines(type) {

    goodsLineCounters[type] = 0;

    const container =
        document.getElementById(`${type}-lines`);

    if (!container) {
        return;
    }

    container.innerHTML = "";

    addGoodsLine(type);
}

function addGoodsLine(type) {

    const rowIndex =
        goodsLineCounters[type]++;

    const container =
        document.getElementById(`${type}-lines`);

    if (!container) {
        return;
    }

    container.insertAdjacentHTML(
        "beforeend",
        goodsLineRowHtml(type, rowIndex)
    );

    initMaterialCombobox(`${type}-line-${rowIndex}`);
}

function removeGoodsLine(type, rowIndex) {

    const container =
        document.getElementById(`${type}-lines`);

    const row =
        document.getElementById(`${type}-line-${rowIndex}-row`);

    if (!container || !row) {
        return;
    }

    // Always keep at least one line on the form.
    if (container.children.length <= 1) {
        return;
    }

    row.remove();
}

function collectGoodsLines(type, message) {

    const materialIDPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;

    const rows =
        Array.from(
            document.querySelectorAll(`#${type}-lines .line-item`)
        );

    if (rows.length === 0) {

        message.textContent =
            "Please add at least one material.";

        return null;
    }

    const lines = [];
    const seenIDs = new Set();

    for (const row of rows) {

        const materialID =
            row.querySelector(
                "input[id$='-material-id']"
            ).value.trim();

        const quantity =
            row.querySelector(
                "input[id$='-quantity']"
            ).value;

        if (!materialIDPattern.test(materialID)) {

            message.textContent =
                "Please select a valid Material ID (from the list) for every line.";

            return null;
        }

        if (!quantity ||
            Number(quantity) <= 0) {

            message.textContent =
                `Quantity must be greater than zero for material ${materialID}.`;

            return null;
        }

        if (seenIDs.has(materialID)) {

            message.textContent =
                `Material ${materialID} appears more than once. Combine it into a single line.`;

            return null;
        }

        seenIDs.add(materialID);

        lines.push({
            materialID: materialID,
            quantity: Number(quantity)
        });
    }

    return lines;
}

async function submitGoodsLines(endpoint, warehouseID, comment, lines) {

    const results = [];

    for (const line of lines) {

        const payload = {

            warehouseID:
                Number(warehouseID),

            materialID:
                line.materialID,

            quantity:
                line.quantity,

            comment:
                comment
        };

        try {

            const response =
                await fetch(
                    endpoint,
                    {
                        method: "POST",

                        headers: {
                            "Content-Type":
                                "application/json"
                        },

                        body:
                            JSON.stringify(payload)
                    }
                );

            const responseText =
                await response.text();

            results.push({
                materialID: line.materialID,
                ok: response.ok,
                detail: responseText
            });
        }
        catch (error) {

            console.error(error);

            results.push({
                materialID: line.materialID,
                ok: false,
                detail: "Could not connect to the server."
            });
        }
    }

    return results;
}

function reportGoodsLinesResult(message, results, label) {

    const succeeded =
        results.filter(result => result.ok);

    const failed =
        results.filter(result => !result.ok);

    if (failed.length === 0) {

        message.textContent =
            `${label} completed successfully for ${succeeded.length} material(s).`;

        return;
    }

    message.innerHTML =
        `${succeeded.length} of ${results.length} material(s) processed successfully.<br>` +
        "Errors:<br>" +
        failed.map(result =>
            `${escapeHtml(result.materialID)}: ${escapeHtml(result.detail)}`
        ).join("<br>");
}

// ============================================================
// TRANSFER MATERIAL FORM
// ============================================================

function showTransferMaterial() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Transfer Material
            </h2>


            <label>
                Source Warehouse ID
            </label>

            <select id="transfer-source"></select>


            <label>
                Destination Warehouse ID
            </label>

            <select id="transfer-destination"></select>


            <label>
                Material ID
            </label>

            ${materialComboboxHtml("transfer")}


            <label>
                Quantity
            </label>

            <input
                type="number"
                id="transfer-quantity"
                min="0.001"
                step="any"
                placeholder="Enter Quantity"
            >

            <label>
                Comment
            </label>

            <textarea
                id="transfer-comment"
                rows="4"
                placeholder="Reason or additional information"></textarea>

            <div class="form-actions">

                <button
                    onclick="transferMaterial()">

                    Transfer Material

                </button>

            </div>


            <div id="transfer-message">
            </div>

        </div>
    `;

    initMaterialCombobox("transfer");

    populateWarehouseSelect("transfer-source", "", WH_OPTS);

    populateWarehouseSelect("transfer-destination", "", WH_OPTS);
}

// ============================================================
// TRANSFER MATERIAL
// ============================================================

async function transferMaterial() {

    const sourceWarehouseID =
        document.getElementById(
            "transfer-source"
        ).value;

    const destinationWarehouseID =
        document.getElementById(
            "transfer-destination"
        ).value;

    const materialID =
        document.getElementById(
            "transfer-material-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "transfer-quantity"
        ).value;

    const comment =
        document.getElementById(
            "transfer-comment"
        ).value.trim();

    const message =
        document.getElementById(
            "transfer-message"
        );

    const materialIDPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!sourceWarehouseID ||
        !destinationWarehouseID) {

        message.textContent =
            "Please enter both warehouse IDs.";

        return;
    }


    if (
        Number(sourceWarehouseID) ===
        Number(destinationWarehouseID)
    ) {

        message.textContent =
            "Source and destination warehouses must be different.";

        return;
    }


    if (!materialIDPattern.test(materialID)) {

        message.textContent =
            "Invalid Material ID. Expected format ###-###### or ######-00.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    const transfer = {

        sourceWarehouseID:
            Number(sourceWarehouseID),

        destinationWarehouseID:
            Number(destinationWarehouseID),

        materialID:
            materialID,

        quantity:
            Number(quantity),

        comment:
            comment
    };

    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/inventory/transfer",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(transfer)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Material transferred successfully.";

            clearInputFields([
                "transfer-source",
                "transfer-destination",
                "transfer-material-id",
                "transfer-material-id-search",
                "transfer-quantity",
                "transfer-comment"
            ]);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY INVENTORY
// ============================================================

async function displayInventory() {

    const content =
        document.getElementById(
            "inventory-content"
        );


    content.innerHTML = `
        <p>
            Loading inventory...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/inventory"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${message}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.warehouses ||
            data.warehouses.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">

                    No inventory available.

                </div>
            `;

            return;
        }


        let html = `
            <div class="warehouse-toggle-all">
                <button class="button-muted" onclick="setAllWarehousesCollapsed(true)">
                    Collapse all
                </button>
                <button class="button-muted" onclick="setAllWarehousesCollapsed(false)">
                    Expand all
                </button>
            </div>
        `;


        for (
            const warehouse of data.warehouses
        ) {

            html += `

                <div class="material-table-container warehouse-block">

                    <h2 class="warehouse-heading" onclick="toggleWarehouseBlock(this)">
                        <span class="warehouse-caret">&#9660;</span>
                        Warehouse
                        ${warehouse.id}
                        -
                        ${escapeHtml(
                            warehouse.name
                        )}
                        <small class="warehouse-count">
                            (${(warehouse.inventory || []).length} material(s))
                        </small>
                    </h2>

                    <div class="warehouse-body">
            `;


            if (
                !warehouse.inventory ||
                warehouse.inventory.length === 0
            ) {

                html += `

                    <div class="empty-message">

                        Warehouse is empty.

                    </div>

                `;

            }
            else {

                html += `

                    <table class="material-table">

                        <thead>

                            <tr>

                                <th>
                                    Material ID
                                </th>

                                <th>
                                    Material Name
                                </th>

                                <th>
                                    Quantity
                                </th>

                                <th>
                                    UoM
                                </th>

                            </tr>

                        </thead>

                        <tbody>
                `;


                for (
                    const item of warehouse.inventory
                ) {

                    html += `

                        <tr>

                            <td>
                                ${escapeHtml(
                                    item.materialID
                                )}
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.materialName
                                )}
                            </td>

                            <td>
                                ${item.quantity}
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.uom
                                )}
                            </td>

                        </tr>

                    `;
                }


                html += `

                        </tbody>

                    </table>

                `;
            }


            html += `

                    </div>

                </div>

            `;
        }


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// Display Products: collapse a product to one line / open it (click its title)
function toggleProductBlock(heading) {

    heading.closest(".product-block").classList.toggle("collapsed");
}

function setAllProductsCollapsed(collapsed) {

    for (const block of document.querySelectorAll(".product-block")) {
        block.classList.toggle("collapsed", collapsed);
    }
}

// Collapse / expand the inventory of one warehouse (click on its title)
function toggleWarehouseBlock(heading) {

    const block = heading.closest(".warehouse-block");

    block.classList.toggle("collapsed");
}

function setAllWarehousesCollapsed(collapsed) {

    for (const block of document.querySelectorAll(".warehouse-block")) {
        block.classList.toggle("collapsed", collapsed);
    }
}

// ============================================================
// INVENTORY CHECK
// ============================================================

async function inventoryCheck() {

    const content =
        document.getElementById(
            "inventory-content"
        );

    content.innerHTML = `
        <p>
            Checking inventory...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/inventory/check"
            );


        const data =
            await response.json();


        if (!response.ok) {

            content.innerHTML = `

                <div class="empty-message">

                    Inventory check failed.

                    <br><br>

                    ${escapeHtml(
                        data.message || ""
                    )}

                </div>

            `;

            return;
        }


        // ----------------------------------------------------
        // Inventory OK
        // ----------------------------------------------------

        if (data.valid) {

            content.innerHTML = `

                <div class="search-result">

                    <h3>
                        ✓ Inventory Check Passed
                    </h3>

                    <p>
                        No inconsistencies were found.
                    </p>

                    <p>
                        Warehouses checked:
                        <strong>
                            ${data.warehousesChecked}
                        </strong>
                    </p>

                    <p>
                        Inventory entries checked:
                        <strong>
                            ${data.entriesChecked}
                        </strong>
                    </p>

                </div>

            `;

            return;
        }


        // ----------------------------------------------------
        // Problems found
        // ----------------------------------------------------

        let html = `

            <div class="search-result">

                <h3>
                    Inventory Check
                </h3>

                <p>
                    The following inconsistencies were found:
                </p>

                <ul>
        `;


        for (
            const problem of data.problems
        ) {

            html += `

                <li>
                    ${escapeHtml(problem)}
                </li>

            `;
        }


        html += `

                </ul>

            </div>

        `;


        content.innerHTML = html;

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `

            <p>
                Could not connect to the server.
            </p>

        `;
    }
}

// ============================================================
// CREATE PRODUCT FORM
// ============================================================

function showCreateProduct() {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Create Product
            </h2>


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <label>
                Product Name
            </label>

            <input
                type="text"
                id="product-name"
                placeholder="Enter Product Name"
            >


            <label>
                Description
            </label>

            <textarea
                id="product-description"
                rows="4"
                placeholder="Enter Product Description"></textarea>


            <label>
                Main Warehouse
            </label>

            <select id="product-main-warehouse">
                <option value="">
                    Loading warehouses...
                </option>
            </select>

            <small>
                Optional. Display Products shows only this
                Warehouse's stock for this Product instead of every
                Warehouse - can be set or changed later from Modify
                Product.
            </small>


            <h3>
                Bill of Materials
            </h3>


            <div id="bom-items">
            </div>


            <button
                type="button"
                onclick="addBOMItemRow()">

                + Add Component

            </button>


            <div class="form-actions">

                <button
                    onclick="createProduct()">

                    Create Product

                </button>

            </div>


            <div id="product-message">
            </div>

        </div>
    `;

    bomItemCounter = 0;

    populateWarehouseSelect("product-main-warehouse");
}

// ============================================================
// ADD BOM ITEM
// ============================================================
// Same Material search combobox used everywhere else (Transfer
// Material, Goods Receipt/Issue, New Procurement Order) - lets a
// component be found by typing part of its ID or name instead of
// having to remember the exact Material ID.

let bomItemCounter = 0;

function bomItemRowHtml(prefix) {

    return `
        <div class="bom-row" id="${prefix}-row">

            ${materialComboboxHtml(prefix, "Search by ID or name...")}

            <input
                type="number"
                class="bom-quantity"
                min="0.001"
                step="any"
                placeholder="Quantity"
            >

            <button
                type="button"
                onclick="document.getElementById('${prefix}-row').remove()">

                Remove

            </button>

        </div>
    `;
}

function addBOMItemRow() {

    const container =
        document.getElementById(
            "bom-items"
        );

    const rowIndex =
        bomItemCounter++;

    const prefix =
        `bom-item-${rowIndex}`;

    const wrapper =
        document.createElement("div");

    wrapper.innerHTML =
        bomItemRowHtml(prefix).trim();

    container.appendChild(
        wrapper.firstElementChild
    );

    initMaterialCombobox(prefix);
}

// ============================================================
// CREATE PRODUCT
// ============================================================

async function createProduct() {

    const id =
        document.getElementById(
            "product-id"
        ).value.trim();

    const name =
        document.getElementById(
            "product-name"
        ).value.trim();

    const description =
        document.getElementById(
            "product-description"
        ).value.trim();

    const mainWarehouseID =
        Number(
            document.getElementById(
                "product-main-warehouse"
            ).value
        ) || 0;


    const message =
        document.getElementById(
            "product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    // --------------------------------------------------------
    // Validate Product ID
    // --------------------------------------------------------

    if (!productIDPattern.test(id)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    // --------------------------------------------------------
    // Validate required fields
    // --------------------------------------------------------

    if (!name) {

        message.textContent =
            "Product Name is required.";

        return;
    }


    // --------------------------------------------------------
    // Read BOM rows
    // --------------------------------------------------------

    const rows =
        document.querySelectorAll(
            "#bom-items .bom-row"
        );


    const bom = [];


    for (const row of rows) {

        const materialIDInput =
            row.querySelector(
                'input[type="hidden"][id$="-material-id"]'
            );

        const materialID =
            materialIDInput ?
            materialIDInput.value.trim() : "";


        const quantity =
            Number(
                row.querySelector(
                    ".bom-quantity"
                ).value
            );


        if (!/^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/.test(materialID))
        {
            const typedText =
                (row.querySelector('input[type="text"]') || {}).value || "";

            message.textContent =
                typedText.trim()
                ? `"${typedText.trim()}" is not a selected Material - ` +
                  "please pick it from the dropdown list."
                : "A BOM row has no Material selected - pick one " +
                  "from the list or remove the row.";

            return;
        }


        if (quantity <= 0)
        {
            message.textContent =
                "BOM quantity must be greater than zero.";

            return;
        }


        if (!hasMaxThreeDecimals(quantity))
        {
            message.textContent =
                "BOM quantity can have at most 3 decimals.";

            return;
        }


        bom.push({

            materialID:
                materialID,

            quantity:
                quantity

        });
    }


    // --------------------------------------------------------
    // Create product object
    // --------------------------------------------------------

    const product = {

        id: id,

        name: name,

        description: description,

        mainWarehouseID: mainWarehouseID,

        bom: bom

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/products/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(product)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product created successfully.";

            clearInputFields([
                "product-id",
                "product-name",
                "product-description",
                "product-main-warehouse"
            ]);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// MODIFY PRODUCT FORM
// ============================================================

function showModifyProduct(statusMessage) {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Modify Product
            </h2>

            ${statusMessage ?
                `<p class="success-message">${escapeHtml(statusMessage)}</p>` :
                ""}

            <label>
                Product ID
            </label>

            ${productComboboxHtml("modify-lookup")}

            <div class="form-actions">

                <button onclick="loadProductForModify()">
                    Load Product
                </button>

            </div>

            <div id="modify-product-form">
            </div>

        </div>
    `;

    initProductCombobox("modify-lookup");
}

// ============================================================
// ADD BOM ITEM (MODIFY PRODUCT)
// ============================================================

let modifyBomItemCounter = 0;

function modifyBomItemRowHtml(prefix, quantity) {

    return `
        <div class="bom-row modify-bom-row" id="${prefix}-row">

            ${materialComboboxHtml(prefix, "Search by ID or name...")}

            <input
                type="number"
                class="modify-bom-quantity"
                min="0.001"
                step="any"
                placeholder="Quantity"
                value="${quantity || ""}"
            >

            <button
                type="button"
                onclick="document.getElementById('${prefix}-row').remove()">

                Remove

            </button>

        </div>
    `;
}

function addModifyBOMItemRow(materialID, quantity) {

    const container =
        document.getElementById(
            "modify-bom-items"
        );

    const rowIndex =
        modifyBomItemCounter++;

    const prefix =
        `modify-bom-item-${rowIndex}`;

    const wrapper =
        document.createElement("div");

    wrapper.innerHTML =
        modifyBomItemRowHtml(prefix, quantity).trim();

    container.appendChild(
        wrapper.firstElementChild
    );

    initMaterialCombobox(prefix, materialID);
}

// ============================================================
// LOAD PRODUCT FOR MODIFY
// ============================================================

async function loadProductForModify() {

    const id =
        document.getElementById(
            "modify-lookup-product-id"
        ).value.trim();

    const modifyForm =
        document.getElementById(
            "modify-product-form"
        );

    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;

    if (!productIDPattern.test(id)) {

        modifyForm.innerHTML = `
            <p>
                Please select a valid Product from the list.
            </p>
        `;

        return;
    }

    modifyForm.innerHTML = `
        <p>
            Loading product...
        </p>
    `;

    try {

        const response =
            await fetch(
                `/api/products/${encodeURIComponent(id)}`
            );

        const data =
            await response.json();

        if (!response.ok) {

            modifyForm.innerHTML = `
                <p>
                    ${data.message}
                </p>
            `;

            return;
        }

        modifyForm.innerHTML = `

            <input type="hidden" id="modify-product-id" value="${escapeHtml(data.id)}">

            <p>
                Product ID: <strong>${escapeHtml(data.id)}</strong>
            </p>

            <label>
                Product Name
            </label>

            <input
                type="text"
                id="modify-product-name"
                value="${escapeHtml(data.name)}"
            >

            <label>
                Description
            </label>

            <textarea
                id="modify-product-description"
                rows="4"
            >${escapeHtml(data.description || "")}</textarea>

            <label>
                Main Warehouse
            </label>

            <select id="modify-product-main-warehouse">
                <option value="">
                    Loading warehouses...
                </option>
            </select>

            <small>
                Optional. Display Products shows only this
                Warehouse's stock for this Product instead of every
                Warehouse.
            </small>

            <h3>
                Bill of Materials
            </h3>

            <div id="modify-bom-items">
            </div>

            <button
                type="button"
                onclick="addModifyBOMItemRow()">

                + Add Component

            </button>

            <div class="form-actions">

                <button onclick="modifyProductSubmit()">
                    Save Changes
                </button>

            </div>

            <div id="modify-product-message">
            </div>
        `;

        modifyBomItemCounter = 0;

        const bom =
            data.bom || [];

        for (const item of bom) {

            addModifyBOMItemRow(
                item.materialID,
                item.quantity);
        }

        populateWarehouseSelect(
            "modify-product-main-warehouse",
            data.mainWarehouseID);
    }
    catch (error) {

        console.error(error);

        modifyForm.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// MODIFY PRODUCT (SUBMIT)
// ============================================================

async function modifyProductSubmit() {

    const id =
        document.getElementById(
            "modify-product-id"
        ).value;

    const name =
        document.getElementById(
            "modify-product-name"
        ).value.trim();

    const description =
        document.getElementById(
            "modify-product-description"
        ).value.trim();

    const mainWarehouseID =
        Number(
            document.getElementById(
                "modify-product-main-warehouse"
            ).value
        ) || 0;

    const message =
        document.getElementById(
            "modify-product-message"
        );

    if (!name) {

        message.textContent =
            "Product Name is required.";

        return;
    }

    const rows =
        document.querySelectorAll(
            "#modify-bom-items .modify-bom-row"
        );

    const bom = [];

    for (const row of rows) {

        const materialIDInput =
            row.querySelector(
                'input[type="hidden"][id$="-material-id"]'
            );

        const materialID =
            materialIDInput ?
            materialIDInput.value.trim() : "";

        const quantity =
            Number(
                row.querySelector(
                    ".modify-bom-quantity"
                ).value
            );

        if (!/^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/.test(materialID)) {

            const typedText =
                (row.querySelector('input[type="text"]') || {}).value || "";

            message.textContent =
                typedText.trim()
                ? `"${typedText.trim()}" is not a selected Material - ` +
                  "please pick it from the dropdown list."
                : "A BOM row has no Material selected - pick one " +
                  "from the list or remove the row.";

            return;
        }

        if (quantity <= 0) {

            message.textContent =
                "BOM quantity must be greater than zero.";

            return;
        }

        if (!hasMaxThreeDecimals(quantity)) {

            message.textContent =
                "BOM quantity can have at most 3 decimals.";

            return;
        }

        bom.push({
            materialID: materialID,
            quantity: quantity
        });
    }

    const product = {
        id: id,
        name: name,
        description: description,
        mainWarehouseID: mainWarehouseID,
        bom: bom
    };

    try {

        const response =
            await fetch(
                "/api/products/modify",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify(product)
                }
            );

        const responseText =
            await response.text();

        if (response.ok) {

            // Re-render the form empty (Product ID field reset, no
            // fields filled in) so it is visually obvious the change
            // was saved, rather than leaving the just-saved values on
            // screen looking unchanged.
            showModifyProduct(
                "Product successfully modified.");

            return;
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DISPLAY PRODUCTS
// ============================================================

async function showDisplayProducts() {

    const content =
        document.getElementById(
            "product-content"
        );


    content.innerHTML = `
        <p>
            Loading products...
        </p>
    `;


    try {

        const response =
            await fetch(
                "/api/products"
            );


        if (!response.ok) {

            const message =
                await response.text();

            content.innerHTML = `
                <p>
                    Error: ${escapeHtml(message)}
                </p>
            `;

            return;
        }


        const data =
            await response.json();


        if (
            !data.products ||
            data.products.length === 0
        ) {

            content.innerHTML = `
                <div class="empty-message">
                    No products available.
                </div>
            `;

            return;
        }


        const safetyStockUnits =
            Number(data.safetyStockUnits) || 10;


        let html = "";


        for (
            const product of data.products
        ) {

            html += `

                <div class="material-table-container material-table-container-fit product-block collapsed">

                    <div class="product-display-head">

                        <div class="product-display-photo">
                            ${product.photo ?
                                `<img src="${escapeHtml(product.photo)}" alt="">` :
                                `<span class="product-tile-noimg">No photo</span>`}
                        </div>

                        <div>
                    <h2 class="product-heading" onclick="toggleProductBlock(this)">
                        <span class="warehouse-caret">&#9660;</span>
                        ${escapeHtml(product.id)}
                        -
                        ${escapeHtml(product.name)}
                        <small class="warehouse-count">
                            (${(product.bom || []).length} component(s))
                        </small>
                    </h2>

                    <div class="product-extra">

                    <p>
                        ${escapeHtml(
                            product.description || ""
                        )}
                    </p>

                    ${
                        product.mainWarehouseName
                        ? `<p class="product-main-warehouse">
                                Main Warehouse:
                                <strong>${escapeHtml(product.mainWarehouseName)}</strong>
                           </p>`
                        : `<p class="product-main-warehouse product-main-warehouse-unset">
                                No Main Warehouse set - showing every warehouse below.
                                Set one from Modify Product.
                           </p>`
                    }
                    <label class="product-tile-upload">
                        ${product.photo ? "Change photo" : "Add photo"}
                        <input type="file" accept="image/*" style="display:none;"
                            onchange="uploadProductPhotoById('${escapeHtml(product.id)}', this, showDisplayProducts)">
                    </label>

                    </div>

                        </div>
                    </div>

                    <div class="product-body">
            `;


            if (
                !product.bom ||
                product.bom.length === 0
            ) {

                html += `

                    <div class="empty-message">
                        No BOM components.
                    </div>

                `;

            }
            else {

                html += `

                    <table class="material-table material-table-sticky">

                        <thead>

                            <tr>

                                <th>
                                    Photo
                                </th>

                                <th>
                                    Material ID
                                </th>

                                <th>
                                    Name
                                </th>

                                <th>
                                    Drawing Number
                                </th>

                                <th>
                                    Quantity
                                </th>

                                <th>
                                    Stock by Warehouse
                                    (Safety Stock: ${safetyStockUnits} un)
                                </th>

                            </tr>

                        </thead>

                        <tbody>
                `;


                for (
                    const item of product.bom
                ) {

                    // Internal Supplier materials are self-
                    // manufactured, not purchased or held as Warehouse
                    // stock (see Supplier Management) - greyed out and
                    // with no stock column, instead of a misleading
                    // "0" that reads like a shortage.

                    const isInternal =
                        !!item.hasInternalSupplier;

                    const warehouseStock =
                        item.warehouseStock || [];

                    const stockCell =
                        isInternal
                            ? `<small>Internal Supplier - not stocked</small>`
                            : (warehouseStock.length === 0
                                ? "<small>No warehouses.</small>"
                                : warehouseStock.map(stock => {

                                    const color =
                                        stock.sufficientForSafetyStock
                                            ? "#15803d"
                                            : "#b91c1c";

                                    return `
                                        <div style="white-space: nowrap;">
                                            ${escapeHtml(stock.warehouseName)}:
                                            <strong style="color: ${color};">
                                                ${stock.quantity}${item.uom ? " " + escapeHtml(item.uom) : ""}
                                            </strong>
                                        </div>
                                    `;
                                }).join(""));

                    html += `

                        <tr class="${isInternal ? "product-bom-internal" : ""}">

                            <td>
                                ${
                                    item.photo
                                    ? `<img
                                            src="/${item.photo}"
                                            class="material-thumbnail"
                                            alt="Material photo"
                                       >`
                                    : "No photo"
                                }
                            </td>

                            <td>
                                <a
                                    href="#"
                                    class="material-id-link"
                                    title="View / modify this Material"
                                    onclick="event.preventDefault(); openMaterialFromProduct('${escapeHtml(item.materialID)}');">

                                    ${escapeHtml(item.materialID)}

                                </a>
                            </td>

                            <td>
                                ${escapeHtml(
                                    item.materialName || ""
                                )}
                            </td>

                            <td class="product-bom-drawing-number">
                                ${escapeHtml(item.drawingNumber || "")}
                            </td>

                            <td>
                                ${item.quantity}${item.uom ? " " + escapeHtml(item.uom) : ""}
                            </td>

                            <td>
                                ${stockCell}
                            </td>

                        </tr>

                    `;
                }


                html += `

                        </tbody>

                    </table>

                `;
            }


            html += `
                    </div>
                </div>
            `;
        }


        content.innerHTML = `
            <div class="warehouse-toggle-all">
                <button class="button-muted" onclick="setAllProductsCollapsed(true)">
                    Collapse all
                </button>
                <button class="button-muted" onclick="setAllProductsCollapsed(false)">
                    Expand all
                </button>
            </div>
        ` + html;

        enableColumnFilters(content, { skip: /^(photo|stock by)/i });

    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>
                Could not connect to the server.
            </p>
        `;
    }
}

// ============================================================
// PRODUCT SETTINGS
// ============================================================

async function loadProductSafetyStockUnits() {

    try {

        const response =
            await fetch("/api/products/config");

        const data =
            await response.json();

        return Number(data.safetyStockUnits) || 10;
    }
    catch (error) {

        console.error(error);

        return 10;
    }
}

async function showProductSettings() {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `
        <p>Loading Settings...</p>
    `;

    const safetyStockUnits =
        await loadProductSafetyStockUnits();

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Product Settings
            </h2>

            <label>
                Safety Stock (units)
            </label>

            <small>
                In Display Products, each BOM material's stock at a
                Warehouse is shown in green when it is enough to build
                this many units of the Product there, and in red
                otherwise. Whenever we talk about "Safety Stock" in
                this system, this is the number we mean.
            </small>

            <input
                type="number"
                min="1"
                step="1"
                id="productsettings-safetystock"
                value="${safetyStockUnits}"
            >

            <div class="form-actions">
                <button onclick="submitProductSafetyStock()">
                    Save
                </button>
            </div>

            <div id="productsettings-message">
            </div>

        </div>
    `;
}

async function submitProductSafetyStock() {

    const message =
        document.getElementById(
            "productsettings-message"
        );

    const units =
        Number(
            document.getElementById(
                "productsettings-safetystock"
            ).value
        );

    if (!units || units <= 0 || !Number.isInteger(units)) {

        message.textContent =
            "Safety Stock must be a whole number greater than zero.";

        return;
    }

    try {

        const response =
            await fetch(
                "/api/products/config",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({ safetyStockUnits: units })
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        message.textContent =
            "Safety Stock updated.";
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DELETE PRODUCT FORM
// ============================================================

function showDeleteProduct() {

    const content =
        document.getElementById(
            "product-content"
        );


    content.innerHTML = `

        <div class="form-container">

            <h2>
                Delete Product
            </h2>


            <label>
                Product ID
            </label>

            <input
                type="text"
                id="delete-product-id"
                placeholder="###-######"
                maxlength="10"
            >


            <div class="form-actions">

                <button
                    onclick="deleteProduct()">

                    Delete Product

                </button>

            </div>


            <div id="delete-product-message">
            </div>

        </div>
    `;
}

// ============================================================
// DELETE PRODUCT
// ============================================================

async function deleteProduct() {

    const id =
        document.getElementById(
            "delete-product-id"
        ).value.trim();


    const message =
        document.getElementById(
            "delete-product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    if (!productIDPattern.test(id)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    try {

        const response =
            await fetch(
                "/api/products/delete",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            id: id
                        })
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product deleted successfully.";

            document.getElementById(
                "delete-product-id"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// SELL PRODUCT FORM
// ============================================================

function showSellProduct() {

    const content =
        document.getElementById(
            "product-content"
        );

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Sell Product
            </h2>


            <label>
                Warehouse ID
            </label>

            <select id="sell-warehouse-id"></select>


            <label>
                Product ID
            </label>

            <select id="sell-product-id"></select>


            <label>
                Quantity Sold
            </label>

            <input
                type="number"
                id="sell-quantity"
                min="1"
                placeholder="Enter Quantity"
            >


            <label>
                Comment (optional)
            </label>

            <textarea
                id="sell-comment"
                rows="3"
                placeholder="Identify this sale: customer, order number, delivery note..."></textarea>


            <div class="form-actions">

                <button
                    onclick="sellProduct()">

                    Sell Product

                </button>

            </div>


            <div id="sell-product-message">
            </div>

        </div>
    `;

    populateWarehouseSelect("sell-warehouse-id", "", WH_OPTS);

    populateSellProductSelect();
}

// Product list for Sell Product: "ID - Name"
async function populateSellProductSelect() {

    const select = document.getElementById("sell-product-id");

    if (!select) {
        return;
    }

    try {

        const response = await fetch("/api/products");

        const data = await response.json();

        const products = data.products || [];

        select.innerHTML =
            `<option value="">Select Product</option>` +
            products.map(product => `
                <option value="${escapeHtml(product.id)}">
                    ${escapeHtml(product.id)} - ${escapeHtml(product.name)}
                </option>`).join("");
    }
    catch (error) {

        console.error(error);

        select.innerHTML =
            `<option value="">Could not load products</option>`;
    }
}

// ============================================================
// SELL PRODUCT
// ============================================================

async function sellProduct() {

    const warehouseID =
        document.getElementById(
            "sell-warehouse-id"
        ).value;

    const productID =
        document.getElementById(
            "sell-product-id"
        ).value.trim();

    const quantity =
        document.getElementById(
            "sell-quantity"
        ).value;

    const comment =
        document.getElementById(
            "sell-comment"
        ).value.trim();


    const message =
        document.getElementById(
            "sell-product-message"
        );


    const productIDPattern =
        /^[0-9]{3}-[0-9]{6}$/;


    // --------------------------------------------------------
    // Validation
    // --------------------------------------------------------

    if (!warehouseID) {

        message.textContent =
            "Please enter a Warehouse ID.";

        return;
    }


    if (!productID) {

        message.textContent =
            "Please select a Product.";

        return;
    }


    if (!productIDPattern.test(productID)) {

        message.textContent =
            "Invalid Product ID. Expected format ###-######.";

        return;
    }


    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity must be greater than zero.";

        return;
    }


    const sale = {

        warehouseID:
            Number(warehouseID),

        productID:
            productID,

        quantity:
            Number(quantity),

        comment:
            comment

    };


    // --------------------------------------------------------
    // Send to C++
    // --------------------------------------------------------

    try {

        const response =
            await fetch(
                "/api/products/sell",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(sale)
                }
            );


        const responseText =
            await response.text();


        if (response.ok) {

            message.textContent =
                "Product sold successfully.";

            document.getElementById(
                "sell-quantity"
            ).value = "";

            document.getElementById(
                "sell-comment"
            ).value = "";

        }
        else {

            message.textContent =
                "Error: " + responseText;
        }

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// READ FILE AS BASE64
// ============================================================

function readFileAsBase64(file) {

    return new Promise(
        (resolve, reject) => {

            const reader =
                new FileReader();

            reader.onload = () => {

                resolve(
                    reader.result
                );
            };

            reader.onerror = () => {

                reject(
                    new Error(
                        "Could not read image file."
                    )
                );
            };

            reader.readAsDataURL(file);
        }
    );
}

// ============================================================
// CLEAR INPUT FIELDS
// ============================================================

function clearInputFields(ids) {

    ids.forEach(id => {

        const element =
            document.getElementById(id);

        if (element) {

            element.value = "";

        }
    });
}

// ============================================================
// SET DATA LOGGING
// ============================================================

async function setDataLogging() {

    const checkbox =
        document.getElementById(
            "log-data-operations"
        );

    const message =
        document.getElementById(
            "data-logging-message"
        );


    try {

        const response =
            await fetch(
                "/api/data/logging",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            enabled:
                                checkbox.checked
                        })
                }
            );


        const data =
            await response.json();


        if (!response.ok) {

            message.textContent =
                data.message ||
                "Could not update logging setting.";

            return;
        }


        message.textContent =
            data.message;

    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// AUTO SAVE AND LOAD DATA
// ============================================================

async function loadAutoSaveAndLoadState() {

    try {

        const response =
            await fetch("/api/data/config");

        const data =
            await response.json();

        if (!response.ok) {
            return;
        }

        const checkbox =
            document.getElementById(
                "auto-save-and-load"
            );

        if (checkbox) {
            checkbox.checked =
                data.autoSaveAndLoad;
        }
    }
    catch (error) {

        console.error(
            "Could not load Auto Save and Load Data state:",
            error
        );
    }
}

async function toggleAutoSaveAndLoad() {

    const checkbox =
        document.getElementById(
            "auto-save-and-load"
        );

    const message =
        document.getElementById(
            "data-logging-message"
        );

    try {

        const response =
            await fetch(
                "/api/data/config",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({
                        autoSaveAndLoad: checkbox.checked
                    })
                }
            );

        const data =
            await response.json();

        if (!response.ok) {

            message.textContent =
                data.message ||
                "Could not update Auto Save and Load Data setting.";

            return;
        }

        message.textContent =
            data.message;
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// DOCUMENTS FOLDER (Procurement Order Confirmation / Lieferschein)
// ============================================================

async function loadDocumentsFolderState() {

    try {

        const response =
            await fetch("/api/settings/documents-folder");

        const data =
            await response.json();

        if (!response.ok) {
            return;
        }

        const input =
            document.getElementById(
                "documents-folder-path"
            );

        if (input) {
            input.value =
                data.documentsFolder || "";
        }
    }
    catch (error) {

        console.error(
            "Could not load Documents Folder setting:",
            error
        );
    }
}

async function saveDocumentsFolder() {

    const folder =
        document.getElementById(
            "documents-folder-path"
        ).value.trim();

    const message =
        document.getElementById(
            "documents-folder-message"
        );

    try {

        const response =
            await fetch(
                "/api/settings/documents-folder",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({
                        documentsFolder: folder
                    })
                }
            );

        const data =
            await response.json();

        if (!response.ok) {

            message.textContent =
                data.message ||
                "Could not save the Documents Folder.";

            return;
        }

        message.textContent =
            folder ?
            "Documents Folder saved." :
            "Documents Folder cleared - uploads are blocked until it is set again.";
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// RESET PRC / INVOICE NUMBERING
// ============================================================

async function resetNumbering() {

    const confirmInput =
        document.getElementById(
            "reset-numbering-confirm"
        );

    const message =
        document.getElementById(
            "reset-numbering-message"
        );

    if (confirmInput.value.trim() !== "RESET") {

        message.textContent =
            "Type RESET (all caps) in the box to confirm.";

        return;
    }

    try {

        const response =
            await fetch(
                "/api/data/reset-numbering",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({
                        confirm: confirmInput.value.trim()
                    })
                }
            );

        if (!response.ok) {

            message.textContent =
                await response.text();

            return;
        }

        const data =
            await response.json();

        message.textContent =
            data.message;

        confirmInput.value = "";
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// BACK TO DASHBOARD
// ============================================================

function closeModule() {

    const dashboard =
        document.querySelector(
            ".dashboard"
        );

    const moduleView =
        document.getElementById(
            "module-view"
        );


    moduleView.classList.add(
        "hidden"
    );

    dashboard.style.display =
        "grid";
}

// ============================================================
// EXIT APPLICATION
// ============================================================
// Saves everything (same as the "Save Data" button) and then shuts
// the backend server down, so the user does not have to go back to
// Visual Studio / the console window to stop it. Because this closes
// the server this page depends on, the response itself may not make
// it back before the connection drops - that is expected, not an
// error, so it is not reported as "Could not connect to the server."
// like every other fetch failure in this app.

async function exitApplication() {

    const confirmed = await showConfirmDialog(
        "This will save all data and shut down the Warehouse " +
        "Management System server. You will need to start it again " +
        "(from Visual Studio) to keep working. Continue?"
    );

    if (!confirmed) {
        return;
    }

    try {

        await fetch(
            "/api/data/exit",
            { method: "POST" }
        );
    }
    catch (error) {

        // Expected: the server may shut down before the response
        // finishes, which surfaces here as a failed fetch.
    }

    document.body.innerHTML = `
        <div class="exit-message">

            <h1>
                Warehouse Management System
            </h1>

            <p>
                Data saved. The server has been shut down - you can
                close this tab.
            </p>

        </div>
    `;
}

// ============================================================
// PRODUCT SEARCH COMBOBOX
// ============================================================
// Same pattern as the Material combobox, used by Production
// Projection to pick a Product by ID or name.

let productCache = null;

async function loadProductCache(forceRefresh) {

    if (productCache && !forceRefresh) {
        return productCache;
    }

    try {

        const response =
            await fetch("/api/products");

        const data =
            await response.json();

        productCache =
            data.products || [];
    }
    catch (error) {

        console.error(error);

        productCache = [];
    }

    return productCache;
}

async function initProductCombobox(prefix, selectedID) {

    await loadProductCache(true);

    const hiddenInput =
        document.getElementById(prefix + "-product-id");

    const searchInput =
        document.getElementById(prefix + "-product-id-search");

    if (!hiddenInput || !searchInput) {
        return;
    }

    hiddenInput.value = selectedID || "";
    searchInput.value = selectedID || "";
}

function renderProductOptions(prefix) {

    const searchInput =
        document.getElementById(prefix + "-product-id-search");

    const optionsBox =
        document.getElementById(prefix + "-product-id-options");

    if (!searchInput || !optionsBox) {
        return;
    }

    const filterText =
        searchInput.value.trim().toLowerCase();

    const products =
        productCache || [];

    const matches =
        products.filter(product =>
            product.id.toLowerCase().includes(filterText) ||
            (product.name || "").toLowerCase().includes(filterText)
        );

    if (matches.length === 0) {

        optionsBox.innerHTML = `
            <div class="combobox-option combobox-empty">
                No products found
            </div>
        `;
    }
    else {

        optionsBox.innerHTML =
            matches.map(product => `
                <div class="combobox-option"
                    onmousedown="selectProductOption('${prefix}', '${escapeHtml(product.id)}')">
                    <strong>${escapeHtml(product.id)}</strong>
                    &nbsp;&mdash;&nbsp;${escapeHtml(product.name)}
                </div>
            `).join("");
    }

    optionsBox.classList.remove("hidden");
}

function handleProductSearchInput(prefix) {

    renderProductOptions(prefix);

    const searchInput =
        document.getElementById(prefix + "-product-id-search");

    const hiddenInput =
        document.getElementById(prefix + "-product-id");

    const products =
        productCache || [];

    const typedText =
        searchInput.value.trim();

    const exactMatch =
        products.find(product =>
            product.id === typedText
        );

    hiddenInput.value =
        exactMatch ? exactMatch.id : "";
}

function selectProductOption(prefix, id) {

    document.getElementById(
        prefix + "-product-id"
    ).value = id;

    document.getElementById(
        prefix + "-product-id-search"
    ).value = id;

    document.getElementById(
        prefix + "-product-id-options"
    ).classList.add("hidden");
}

function hideProductOptionsDelayed(prefix) {

    setTimeout(() => {

        const optionsBox =
            document.getElementById(prefix + "-product-id-options");

        if (optionsBox) {
            optionsBox.classList.add("hidden");
        }

    }, 150);
}

function productComboboxHtml(prefix, placeholder) {

    return `
        <div class="combobox">

            <input
                type="text"
                id="${prefix}-product-id-search"
                autocomplete="off"
                placeholder="${placeholder || "Search by ID or name..."}"
                oninput="handleProductSearchInput('${prefix}')"
                onfocus="renderProductOptions('${prefix}')"
                onblur="hideProductOptionsDelayed('${prefix}')"
            >

            <input type="hidden" id="${prefix}-product-id">

            <div id="${prefix}-product-id-options"
                class="combobox-options hidden">
            </div>

        </div>
    `;
}

// ============================================================
// NEW PROJECTION - FORM
// ============================================================

function showNewProjection() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                New Production Projection
            </h2>

            <label>
                Product
            </label>

            ${productComboboxHtml("projection")}

            <small>
                Stock is checked against this Product's Main Warehouse
                (set from Modify Product). If it has none yet, assign
                one there before creating a Projection.
            </small>

            <label>
                Manufacturing Deadline
            </label>

            <input
                type="date"
                id="projection-deadline"
            >

            <label>
                Quantity to Manufacture
            </label>

            <input
                type="number"
                id="projection-quantity"
                min="1"
                placeholder="Enter Quantity"
            >

            <div class="form-actions">

                <button
                    onclick="createProjectionSubmit()">

                    Calculate &amp; Save Projection

                </button>

            </div>

            <div id="projection-message">
            </div>

        </div>
    `;

    initProductCombobox("projection");
}

// ============================================================
// NEW PROJECTION - SUBMIT
// ============================================================
// Creates a persisted Projection batch on the backend: BOM quantity x
// units to manufacture, checked against the Product's own Main
// Warehouse (chosen automatically, not entered here) and against what
// every other open Projection in that Warehouse already needs. Only
// materials with an actual shortfall are stored.

async function createProjectionSubmit() {

    const productID =
        document.getElementById(
            "projection-product-id"
        ).value.trim();

    const deadline =
        document.getElementById(
            "projection-deadline"
        ).value;

    const quantity =
        document.getElementById(
            "projection-quantity"
        ).value;

    const message =
        document.getElementById(
            "projection-message"
        );


    if (!productID) {

        message.textContent =
            "Please select a valid Product from the list.";

        return;
    }

    if (!quantity ||
        Number(quantity) <= 0) {

        message.textContent =
            "Quantity to Manufacture must be greater than zero.";

        return;
    }


    message.textContent =
        "Calculating...";


    const payload = {

        productID: productID,

        deadline: deadline,

        manufactureQuantity: Number(quantity)
    };


    try {

        const response =
            await fetch(
                "/api/projections/create",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(payload)
                }
            );

        const responseData =
            await response.json()
                .catch(() => null);

        if (response.ok &&
            responseData &&
            responseData.id) {

            message.textContent = "";

            await openProjectionDetail(
                responseData.id);
        }
        else {

            const responseText =
                (responseData && responseData.message) ||
                await response.text()
                    .catch(() => "");

            message.textContent =
                responseText ||
                "Enough stock for all materials - nothing to order.";
        }
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROJECTIONS - LIST
// ============================================================

async function showProjections() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Projections...</p>
    `;

    await displayProjections();
}

async function displayProjections() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    try {

        const response =
            await fetch("/api/projections");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const projections =
            (data.projections || []).filter(
                projection => !projection.completed
            );

        if (projections.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    No active Projections. Use "New Projection" to create one, or check Archived Projections.
                </div>
            `;

            return;
        }

        // Grouped by Product: every Projection of one Product together,
        // collapsible to a single line with the total units required.

        const groups = new Map();

        for (const projection of projections) {

            if (!groups.has(projection.productID)) {

                groups.set(projection.productID, {
                    productID: projection.productID,
                    productName: projection.productName || "",
                    items: []
                });
            }

            groups.get(projection.productID).items.push(projection);
        }

        const sortedGroups =
            [...groups.values()].sort((first, second) =>
                first.productID.localeCompare(second.productID));

        let html = `
            <div class="projection-group-actions">
                <button type="button" class="button-muted"
                    onclick="setAllProjectionGroupsCollapsed(true)">
                    Collapse all
                </button>
                <button type="button" class="button-muted"
                    onclick="setAllProjectionGroupsCollapsed(false)">
                    Expand all
                </button>
            </div>
        `;

        for (const group of sortedGroups) {

            group.items.sort((first, second) =>
                (first.deadline || "").localeCompare(second.deadline || ""));

            const totalUnits =
                roundQty3(group.items.reduce(
                    (sum, item) => sum + Number(item.manufactureQuantity || 0),
                    0));

            html += `
                <div class="projection-group">

                    <div class="projection-group-heading"
                         onclick="toggleProjectionGroup(this)">

                        <span class="warehouse-caret">&#9660;</span>

                        <span class="projection-group-title">
                            ${escapeHtml(group.productID)}
                            &mdash;
                            ${escapeHtml(group.productName)}
                        </span>

                        <span class="projection-group-count">
                            (${group.items.length}
                            projection${group.items.length === 1 ? "" : "s"})
                        </span>

                        <span class="projection-group-total">
                            Total required: ${totalUnits} units
                        </span>

                    </div>

                    <div class="projection-group-body procurement-list">
                        ${group.items.map(renderProjectionCard).join("")}
                    </div>

                </div>
            `;
        }

        content.innerHTML = html;
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// Open Projections: collapse a Product's Projections to one line
function toggleProjectionGroup(heading) {

    heading.closest(".projection-group").classList.toggle("collapsed");
}

function setAllProjectionGroupsCollapsed(collapsed) {

    for (const group of document.querySelectorAll(".projection-group")) {
        group.classList.toggle("collapsed", collapsed);
    }
}

// ============================================================
// PROJECTIONS - ARCHIVE (Completed / produced batches only)
// ============================================================

async function showArchivedProjections() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Archived Projections...</p>
    `;

    await displayArchivedProjections();
}

async function displayArchivedProjections() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    try {

        const response =
            await fetch("/api/projections");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const projections =
            (data.projections || []).filter(
                projection => projection.completed === true
            );

        if (projections.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    No Archived Projections yet.
                </div>
            `;

            return;
        }

        let html =
            `<div class="procurement-list">`;

        for (const projection of projections) {
            html += renderProjectionCard(projection);
        }

        html += `</div>`;

        content.innerHTML = html;
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// ============================================================
// PROJECTIONS - STATUS BADGE CLASS
// ============================================================

function projectionStatusClass(status) {

    if (status === "Cancelled") {
        return "status-inactive";
    }

    if (status === "Fully Ordered" ||
        status === "Completed") {
        return "status-completed";
    }

    return "status-partial";
}

// ============================================================
// PROJECTIONS - ORDER TIMING BADGE CLASS
// ============================================================

function timingStatusClass(status) {

    if (status === "Overdue") {
        return "status-inactive";
    }

    if (status === "On Time") {
        return "status-active";
    }

    return "status-ordered";
}

// ============================================================
// PROJECTIONS - CARD MARKUP
// ============================================================

function renderProjectionCard(projection) {

    const displayStatus =
        projection.cancelled ? "Cancelled" :
        projection.completed ? "Completed" : projection.status;

    return `

        <div
            class="procurement-card projection-card"
            id="projection-${projection.id}"
            onclick="openProjectionDetail('${escapeHtml(projection.id)}')">

            <div class="procurement-card-header">

                <div>
                    <strong>${escapeHtml(projection.id)}</strong>
                    &mdash;
                    ${escapeHtml(projection.productID)}
                    ${escapeHtml(projection.productName || "")}
                </div>

                <span class="status-badge ${projectionStatusClass(displayStatus)}">
                    ${escapeHtml(displayStatus)}
                </span>

            </div>

            <div class="material-detail-grid procurement-grid">

                <div class="detail-field">
                    <div class="detail-label">Warehouse</div>
                    <div class="detail-value">${projection.warehouseID}</div>
                </div>

                <div class="detail-field">
                    <div class="detail-label">Deadline</div>
                    <div class="detail-value">${escapeHtml(projection.deadline || "")}</div>
                </div>

                <div class="detail-field">
                    <div class="detail-label">Qty to Manufacture</div>
                    <div class="detail-value">${projection.manufactureQuantity}</div>
                </div>

                <div class="detail-field">
                    <div class="detail-label">Created</div>
                    <div class="detail-value">${escapeHtml(projection.creationDate || "")}</div>
                </div>

            </div>

        </div>
    `;
}

// ============================================================
// PROJECTIONS - DETAIL (materials to order for this batch)
// ============================================================

async function openProjectionDetail(id) {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Projection...</p>
    `;

    try {

        const response =
            await fetch(`/api/projections/${id}`);

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const projection =
            await response.json();

        const readOnly = !!projection.completed;

        const displayStatus =
            projection.cancelled ? "Cancelled" :
            readOnly ? "Completed" : projection.status;

        let html = `

            <button
                type="button"
                class="back-button"
                onclick="showProjections()">

                &larr; Back to Projections

            </button>

            <div class="material-table-container material-table-container-fit">

                <h2>
                    ${escapeHtml(projection.id)}
                    &mdash;
                    ${escapeHtml(projection.productID)}
                    ${escapeHtml(projection.productName || "")}
                </h2>

                <p>
                    Warehouse ${projection.warehouseID}
                    &mdash; Deadline: ${escapeHtml(projection.deadline || "-")}
                    &mdash; Qty to Manufacture: ${projection.manufactureQuantity}
                    &mdash; Status:
                    <span class="status-badge ${projectionStatusClass(displayStatus)}">
                        ${escapeHtml(displayStatus)}
                    </span>
                </p>

                ${readOnly ? `
                    <p>
                        ${projection.cancelled
                            ? `Cancelled on ${escapeHtml(projection.completionDate || "-")}.
                               Nothing was produced and no BOM materials were issued.`
                            : `Production confirmed: ${projection.producedQuantity} units on ${escapeHtml(projection.completionDate || "-")}.
                               BOM materials were issued from the Warehouse.`}
                    </p>
                    <div class="projection-delete-button-row">
                        <button
                            type="button"
                            class="button-secondary"
                            onclick="exportProjectionToExcel('${escapeHtml(projection.id)}')">

                            Export to Excel

                        </button>
                    </div>
                ` : `
                    <div class="projection-delete-button-row">
                        <button
                            type="button"
                            class="button-secondary"
                            onclick="exportProjectionToExcel('${escapeHtml(projection.id)}')">

                            Export to Excel

                        </button>
                        <button
                            type="button"
                            class="button-danger"
                            onclick="deleteProjectionConfirm('${escapeHtml(projection.id)}')">

                            Cancel Projection

                        </button>
                    </div>
                `}

                <div id="projection-detail-message">
                </div>

                <table class="material-table material-table-sticky projection-detail-table">

                    <thead>

                        <tr>
                            <th></th>
                            <th>Photo</th>
                            <th>Material ID</th>
                            <th>Name</th>
                            <th>UoM</th>
                            <th title="This Product's Main Warehouse - current stock, right now">Stock</th>
                            <th title="How much of this material THIS Projection needs">Required</th>
                            <th title="Shared total: what every open Projection in this Warehouse still needs for this material, minus current stock">Qty to Order</th>
                            <th title="Unit price for the quantity entered in Register Order Qty: the last invoice, or the quotation price tier for that quantity (estimate) - changes with the tier">Unit Price</th>
                            <th title="Unit price x the quantity entered in Register Order Qty - updates as you type">Total</th>
                            <th title="Still-outstanding Procurement Order quantity for this material in this Warehouse - informational, not subtracted from Qty to Order">Ordered</th>
                            <th>Order By</th>
                            <th>Status</th>
                            <th>Register Order Qty</th>
                        </tr>

                    </thead>

                    <tbody>
        `;


        let anySelectable = false;

        // Group rows by Supplier (so the purchasing conversation for
        // one Supplier's materials sits together) - a Material with no
        // Supplier assigned yet sorts into its own "No Supplier" group
        // at the end, never mixed alphabetically with real ones.

        const sortedItems =
            [...(projection.items || [])].sort((a, b) => {

                const supplierA = a.supplierName || "";
                const supplierB = b.supplierName || "";

                if (supplierA === "" && supplierB === "") return 0;
                if (supplierA === "") return 1;
                if (supplierB === "") return -1;

                return supplierA.localeCompare(supplierB);
            });

        let lastSupplierGroup = null;

        for (const item of sortedItems) {

            const supplierGroup =
                item.supplierName || "";

            if (supplierGroup !== lastSupplierGroup) {

                lastSupplierGroup = supplierGroup;

                html += `
                    <tr class="projection-supplier-group-row">
                        <td colspan="9">
                            ${escapeHtml(supplierGroup || "No Supplier")}
                        </td>
                        <td colspan="5"
                            class="projection-supplier-subtotal"
                            data-supplier="${escapeHtml(supplierGroup)}">
                        </td>
                    </tr>
                `;
            }

            const rowID =
                "projdet-" + item.materialID;

            // "Qty to Order" (item.pendingQuantity) is the shared,
            // live total for this material across every open
            // Projection in this Warehouse - it can stay above zero
            // even after this exact row has already been ordered
            // (another Projection may still need more), so it is NOT
            // what decides whether this row can still be selected.
            //
            // That decision uses item.orderRegistered instead: a
            // frozen, backend-persisted flag (see ProjectionManager::
            // registerItemOrderAttempt()) set either when a single
            // order from this Projection already covered this line's
            // requiredQuantity, or once two orders have been placed
            // from it regardless of whether they were enough (a line
            // gets at most two chances here - the rest goes through
            // New Order manually). It stays frozen - it will not flip
            // back even if stock or orders change later - until this
            // Projection is completed.

            // No more frozen "Already ordered" step: every line stays
            // visible and can be ordered again (the Ordered column
            // shows what is already on order).
            const alreadyOrdered = readOnly;

            // What the outstanding orders do not cover yet - the
            // suggested quantity (empty when already covered, so the
            // same amount is not ordered twice by habit).
            const remainingToOrder =
                Math.max(0, roundQty3(
                    item.pendingQuantity - (item.orderedQuantity || 0)));

            const covered =
                item.pendingQuantity > 0 && remainingToOrder <= 0;

            const pendingCell =
                alreadyOrdered
                ? `<span class="status-badge status-active">&#10003; Already ordered</span>`
                : `<strong>${item.pendingQuantity}</strong>`;

            const orderedCell =
                item.orderedQuantity > 0
                ? (covered
                    ? `<span title="Outstanding orders cover the Qty to Order">${item.orderedQuantity} &#10003;</span>`
                    : item.orderedQuantity)
                : "-";

            if (!alreadyOrdered) {
                anySelectable = true;
            }

            html += `

                <tr id="${rowID}-row" data-supplier="${escapeHtml(supplierGroup)}">

                    <td>
                        ${
                            alreadyOrdered
                            ? ""
                            : `<input
                                    type="checkbox"
                                    id="${rowID}-select"
                               >`
                        }
                    </td>

                    <td>
                        ${
                            item.photo
                            ? `<img
                                    src="/${item.photo}"
                                    class="material-thumbnail"
                                    alt="Material photo"
                               >`
                            : "No photo"
                        }
                    </td>

                    <td>
                        <a
                            href="#"
                            class="material-id-link"
                            title="View this Material"
                            onclick="event.preventDefault(); openMaterialFromProjection('${escapeHtml(item.materialID)}', '${escapeHtml(projection.id)}');">

                            ${escapeHtml(item.materialID)}

                        </a>
                    </td>
                    <td>${escapeHtml(item.materialName || "")}</td>
                    <td>${escapeHtml(item.uom || "")}</td>
                    <td>${item.currentStock}</td>
                    <td>${item.requiredQuantity}</td>
                    <td>${pendingCell}</td>
                    <td id="${rowID}-price"><small>-</small></td>
                    <td id="${rowID}-total"><small>-</small></td>
                    <td>${orderedCell}</td>
                    <td>${escapeHtml(item.orderByDate || "-")}</td>

                    <td>
                        <span class="status-badge ${timingStatusClass(item.timingStatus)}">
                            ${escapeHtml(item.timingStatus || "")}
                        </span>
                    </td>

                    <td>
                        ${
                            alreadyOrdered
                            ? "-"
                            : `<input
                                    type="number"
                                    min="0.001"
                                    step="any"
                                    class="procurement-qty-input"
                                    id="${rowID}-qty"
                                    value="${remainingToOrder > 0 ? remainingToOrder : ""}"
                               >`
                        }
                    </td>

                </tr>
            `;
        }


        html += `

                    </tbody>

                </table>

                ${anySelectable ? `

                    <div class="procurement-inline-form">

                        <label for="projdet-order-date">
                            Order Date
                        </label>

                        <input
                            type="date"
                            id="projdet-order-date"
                        >

                        <button
                            type="button"
                            onclick="registerSelectedOrders('${escapeHtml(projection.id)}', '${escapeHtml(projection.productID)}', ${projection.warehouseID})">

                            Register Order

                        </button>

                        <button
                            type="button"
                            class="button-today"
                            onclick="registerSelectedOrdersToday('${escapeHtml(projection.id)}', '${escapeHtml(projection.productID)}', ${projection.warehouseID})">

                            Register Today

                        </button>

                    </div>

                ` : ""}

            </div>

            ${(!readOnly && projection.status === "Fully Ordered") ? `

                <div class="procurement-section procurement-section-standalone">

                    <h3>
                        Confirm Production
                    </h3>

                    <p>
                        Confirming production issues the full Bill of
                        Materials for the produced quantity from the
                        Warehouse (Goods Issue) and archives this
                        Projection into Archived Projections.
                    </p>

                    <div class="procurement-inline-form">

                        <label for="projdet-produced-qty">
                            Produced Quantity
                        </label>

                        <input
                            type="number"
                            min="1"
                            id="projdet-produced-qty"
                            value="${projection.manufactureQuantity}"
                        >

                        <label for="projdet-completion-date">
                            Completion Date
                        </label>

                        <input
                            type="date"
                            id="projdet-completion-date"
                        >

                        <button
                            type="button"
                            onclick="confirmProduction('${escapeHtml(projection.id)}')">

                            Confirm Production

                        </button>

                    </div>

                    <div id="projdet-confirm-message"></div>

                </div>

            ` : ""}
        `;

        content.innerHTML = html;

        fillProjectionPrices(projection, readOnly);
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// Unit price of each material for the quantity typed in "Register Order
// Qty" (last invoice, or the quotation price tier for that quantity as
// an estimate). Unit price and Total follow the quantity live: the total
// instantly, the unit price (price tier) right after the typing stops.
const projectionPriceState = {};

function projectionRowFor(materialID) {
    return "projdet-" + materialID;
}

function renderProjectionRowPrice(materialID, projectionID) {

    const state = projectionPriceState[materialID];

    const rowID = projectionRowFor(materialID);

    const priceCell = document.getElementById(`${rowID}-price`);

    const totalCell = document.getElementById(`${rowID}-total`);

    const qtyInput = document.getElementById(`${rowID}-qty`);

    if (!priceCell || !totalCell || !qtyInput) {
        return;
    }

    const quantity = Number(qtyInput.value);

    if (!state || !state.est) {

        priceCell.innerHTML = `<small>-</small>`;

        totalCell.innerHTML = `<small>-</small>`;

        delete totalCell.dataset.exportValue;

        return;
    }

    const est = state.est;

    if (est.source === "None") {

        priceCell.innerHTML = `<small>No price yet</small>`;

        totalCell.innerHTML = `<small>-</small>`;

        delete totalCell.dataset.exportValue;

        return;
    }

    priceCell.dataset.exportValue =
        Number(est.unitPriceEUR).toFixed(4).replace(/0+$/, "").replace(/\.$/, "");

    priceCell.innerHTML =
        renderCostSourceHtml(est, quantity > 0 ? quantity : state.quantity, "", true) +
        `<br><small><a href="#" onclick="event.preventDefault(); ` +
        `openMaterialFromProjection('${escapeHtml(materialID)}', ` +
        `'${escapeHtml(projectionID)}');">price tiers</a></small>`;

    if (quantity > 0) {

        const total = est.unitPriceEUR * quantity;

        totalCell.dataset.exportValue = total.toFixed(2);

        totalCell.dataset.total = String(total);

        totalCell.innerHTML =
            `<strong>${formatMoney(total)} EUR</strong>` +
            `${est.estimated ? "<br><small>(est.)</small>" : ""}`;
    }
    else {

        totalCell.innerHTML = `<small>-</small>`;

        delete totalCell.dataset.exportValue;

        delete totalCell.dataset.total;
    }
}

// Sum of the Totals: the checked rows if any are checked, else all rows
function updateProjectionGrandTotal() {

    const message = document.getElementById("projection-detail-message");

    if (!message) {
        return;
    }

    const rows = [...document.querySelectorAll('[id^="projdet-"][id$="-total"]')];

    const checkedRows = rows.filter(cell => {

        const box = document.getElementById(
            cell.id.replace(/-total$/, "-select"));

        return box && box.checked;
    });

    const used = checkedRows.length > 0 ? checkedRows : rows;

    const grandTotal =
        used.reduce((sum, cell) => sum + Number(cell.dataset.total || 0), 0);

    // Subtotal per Supplier = what that Supplier's order is expected to
    // cost: its checked rows (or all its rows while nothing is checked).
    const anyChecked = checkedRows.length > 0;

    for (const cell of document.querySelectorAll(".projection-supplier-subtotal")) {

        const supplier = cell.dataset.supplier;

        const supplierCells = used.filter(totalCell => {

            const row = document.getElementById(
                totalCell.id.replace(/-total$/, "-row"));

            return row && row.dataset.supplier === supplier;
        });

        const subtotal =
            supplierCells.reduce(
                (sum, totalCell) => sum + Number(totalCell.dataset.total || 0), 0);

        cell.innerHTML =
            subtotal > 0
            ? `Subtotal${anyChecked ? " (selected)" : ""}: ` +
              `<strong>${formatMoney(subtotal)} EUR</strong>`
            : "";
    }

    message.innerHTML =
        grandTotal > 0
        ? `<p><small>Estimated value of the quantities entered ` +
          `(${checkedRows.length > 0 ? "selected" : "all"} priced materials): ` +
          `<strong>${formatMoney(grandTotal)} EUR</strong></small></p>`
        : "";
}

async function refreshProjectionPrice(projection, materialID) {

    const rowID = projectionRowFor(materialID);

    const qtyInput = document.getElementById(`${rowID}-qty`);

    if (!qtyInput) {
        return;
    }

    const quantity = Number(qtyInput.value);

    const state =
        projectionPriceState[materialID] ||
        (projectionPriceState[materialID] = { est: null, quantity: 0, seq: 0 });

    // Total follows the typing right away with the last known unit price
    renderProjectionRowPrice(materialID, projection.id);

    updateProjectionGrandTotal();

    if (!(quantity > 0)) {
        return;
    }

    const seq = ++state.seq;

    try {

        const response = await fetch(
            `/api/price-estimates?items=${encodeURIComponent(materialID + ":" + quantity)}`);

        if (!response.ok || seq !== state.seq) {
            return;
        }

        const data = await response.json();

        const entry = (data.items || [])[0];

        if (!entry) {
            return;
        }

        state.est = entry.estimate;

        state.quantity = quantity;

        renderProjectionRowPrice(materialID, projection.id);

        updateProjectionGrandTotal();
    }
    catch (error) {

        console.error(error);
    }
}

async function fillProjectionPrices(projection, readOnly) {

    if (readOnly) {
        return;
    }

    for (const key of Object.keys(projectionPriceState)) {
        delete projectionPriceState[key];
    }

    const items = (projection.items || []);

    // Live updates: quantity typed -> new tier price and total
    const timers = {};

    for (const item of items) {

        const rowID = projectionRowFor(item.materialID);

        const qtyInput = document.getElementById(`${rowID}-qty`);

        if (qtyInput) {

            qtyInput.addEventListener("input", () => {

                // Total right away, tier price once the typing stops
                renderProjectionRowPrice(item.materialID, projection.id);

                updateProjectionGrandTotal();

                clearTimeout(timers[item.materialID]);

                timers[item.materialID] = setTimeout(
                    () => refreshProjectionPrice(projection, item.materialID),
                    300);
            });
        }

        const box = document.getElementById(`${rowID}-select`);

        if (box) {
            box.addEventListener("change", updateProjectionGrandTotal);
        }
    }

    // First fill: one request for every row that has a quantity
    // (rows with an empty quantity are priced at their Qty to Order).
    const query = items.map(item => {

        const qtyInput =
            document.getElementById(`${projectionRowFor(item.materialID)}-qty`);

        const quantity =
            qtyInput && Number(qtyInput.value) > 0
                ? Number(qtyInput.value)
                : item.pendingQuantity;

        return quantity > 0 ? `${item.materialID}:${quantity}` : null;

    }).filter(Boolean).join(";");

    if (!query) {
        return;
    }

    try {

        const response = await fetch(
            `/api/price-estimates?items=${encodeURIComponent(query)}`);

        if (!response.ok) {
            return;
        }

        const data = await response.json();

        for (const entry of (data.items || [])) {

            projectionPriceState[entry.materialID] = {
                est: entry.estimate,
                quantity: entry.quantity,
                seq: 0
            };

            renderProjectionRowPrice(entry.materialID, projection.id);
        }

        updateProjectionGrandTotal();
    }
    catch (error) {

        console.error(error);
    }
}

// ============================================================
// PROJECTIONS - CONFIRM PRODUCTION (issue BOM, archive batch)
// ============================================================

// Result of "Confirm Production", shown right under the button (the
// page's top message was far away and easy to miss): red box when it
// is refused, green when it went through.

function showConfirmProductionResult(text, ok) {

    const box =
        document.getElementById("projdet-confirm-message");

    if (!box) {
        return;
    }

    box.textContent = (ok ? "\u2713 " : "\u2717 ") + text;

    box.style.cssText =
        "margin-top: 14px; padding: 12px 16px; border-radius: 8px;" +
        "font-weight: bold; border: 1px solid " +
        (ok ? "#86efac" : "#fca5a5") + ";" +
        "background: " + (ok ? "#dcfce7" : "#fee2e2") + ";" +
        "color: " + (ok ? "#166534" : "#991b1b") + ";";

    box.scrollIntoView({ behavior: "smooth", block: "nearest" });
}

async function confirmProduction(id) {

    // Same function the rest of this one used for its messages, now
    // pointing at the box next to the button.
    const message = {
        set textContent(text) {
            showConfirmProductionResult(text, false);
        }
    };

    const producedQuantity =
        document.getElementById(
            "projdet-produced-qty"
        ).value;

    const completionDate =
        document.getElementById(
            "projdet-completion-date"
        ).value;

    if (!producedQuantity ||
        Number(producedQuantity) <= 0) {

        message.textContent =
            "Please enter a Produced Quantity greater than zero.";

        return;
    }

    if (!completionDate) {

        message.textContent =
            "Please enter a Completion Date.";

        return;
    }

    const confirmed = await showConfirmDialog(
        "This will issue the full Bill of Materials for " +
        producedQuantity +
        " unit(s) from the Warehouse and archive this Projection. Continue?"
    );

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                "/api/projections/complete",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({
                        id: id,
                        producedQuantity: Number(producedQuantity),
                        completionDate: completionDate
                    })
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        await showProjections();

        // The Projection is archived now, so confirm on the list page.
        const listContent =
            document.getElementById("procurement-content");

        if (listContent) {

            const banner = document.createElement("div");

            banner.textContent =
                "\u2713 Production confirmed: " + id + " - " +
                producedQuantity + " unit(s) produced on " +
                completionDate + ". The Bill of Materials was issued " +
                "from the Warehouse and the Projection is now in " +
                "Archived Projections.";

            banner.style.cssText =
                "margin-bottom: 16px; padding: 12px 16px;" +
                "border-radius: 8px; font-weight: bold;" +
                "border: 1px solid #86efac; background: #dcfce7;" +
                "color: #166534;";

            listContent.insertBefore(banner, listContent.firstChild);
        }
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROJECTIONS - JUMP TO MATERIAL
// ============================================================
// The Material ID column on a Projection's table links straight to
// that Material's own detail view (its Supplier, drawing, etc.)
// instead of making the user go hunt for it from Material Manager.
// showMaterialDetail() renders into "#material-content", which only
// exists once the Materials module shell itself is open - so this
// switches modules first, the same as clicking "Materials" in the
// sidebar would, then opens the Material directly.
//
// projectionID is remembered (see materialsReturnProjectionID above)
// so showMaterialDetail() can offer a "Go Back to Projection" button -
// set AFTER openModule(), since openModule() itself always resets it
// to null first.

function openMaterialFromProjection(materialID, projectionID) {

    // Pop-up over the Projection (nothing is left, nothing to return
    // to); the pop-up has its own "Open full page" button
    openMaterialFromQuotation(materialID);
}


function openMaterialFromList(materialID) {

    materialsListScroll =
        window.scrollY || document.documentElement.scrollTop || 0;

    materialsReturnToList = true;

    showMaterialDetail(materialID);
}


// One Back button for every way of arriving at a Material: it returns
// to whatever was open before (Products, a Projection, or the exact
// Materials list at the same scroll position).
async function goBackFromMaterialDetail() {

    if (materialsReturnInventoryCount) {

        const countID = materialsReturnInventoryCount;

        openModule("inventory");

        await openInventoryCount(countID, "", true);

        return;
    }

    if (materialsReturnToProducts) {

        goBackToProductsFromMaterial();

        return;
    }

    if (materialsReturnProjectionID) {

        goBackToProjectionFromMaterial();

        return;
    }

    if (materialsReturnToList && lastMaterialsList) {

        const scroll = materialsListScroll;

        materialsReturnToList = false;

        await renderMaterialsList(
            lastMaterialsList.filterFn,
            lastMaterialsList.headingText,
            lastMaterialsList.emptyText);

        window.scrollTo(0, scroll);

        return;
    }

    displayMaterials();
}


function openMaterialFromProduct(materialID) {

    // Pop-up over the Product view; "Open full page" is inside it
    openMaterialFromQuotation(materialID);
}


function goBackToProductsFromMaterial() {

    openModule("products");

    showDisplayProducts();
}


function goBackToProjectionFromMaterial() {

    const projectionID =
        materialsReturnProjectionID;

    if (!projectionID) {
        return;
    }

    // openModule() would reset materialsReturnProjectionID itself,
    // which is fine here - we already captured the ID we need above
    // before switching modules.

    openModule("procurement");

    openProjectionDetail(projectionID);
}


// ============================================================
// PROJECTIONS - EXPORT TO EXCEL
// ============================================================
// Downloads a CSV (opens directly in Excel, no extra library needed)
// of this Projection's material table. Re-fetches the Projection right
// before building the file so the export always reflects whatever is
// on screen right now (live "Qty to Order", freshly-refreshed shortfall
// items, etc.) rather than a stale copy from when the page first loaded.

function csvEscapeCell(value) {

    const text =
        (value === undefined || value === null) ? "" : String(value);

    if (/[",\n]/.test(text)) {
        return '"' + text.replace(/"/g, '""') + '"';
    }

    return text;
}


function downloadCSV(filename, rows) {

    const csvBody =
        rows
            .map(row => row.map(csvEscapeCell).join(","))
            .join("\r\n");

    // UTF-8 BOM so Excel renders accented characters correctly instead
    // of guessing the wrong encoding.

    const blob =
        new Blob(["﻿" + csvBody], { type: "text/csv;charset=utf-8;" });

    const url =
        URL.createObjectURL(blob);

    const link =
        document.createElement("a");

    link.href = url;
    link.download = filename;

    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);

    URL.revokeObjectURL(url);
}


async function exportProjectionToExcel(id) {

    try {

        const response =
            await fetch(`/api/projections/${id}`);

        if (!response.ok) {

            alert("Could not load this Projection to export it.");

            return;
        }

        const projection =
            await response.json();

        const displayStatus =
            projection.cancelled ? "Cancelled" :
            projection.completed ? "Completed" : projection.status;

        const rows = [
            ["Projection", projection.id],
            ["Product", `${projection.productID} ${projection.productName || ""}`.trim()],
            ["Warehouse", projection.warehouseID],
            ["Deadline", projection.deadline || ""],
            ["Qty to Manufacture", projection.manufactureQuantity],
            ["Status", displayStatus],
            [],
            [
                "Supplier",
                "Material ID",
                "Name",
                "UoM",
                "Stock",
                "Required",
                "Qty to Order",
                "Ordered",
                "Order By",
                "Status"
            ]
        ];

        // Same Supplier-first sort as the on-screen table, so the
        // exported file is already grouped for a per-Supplier purchase
        // order instead of needing to be re-sorted in Excel.

        const sortedExportItems =
            [...(projection.items || [])].sort((a, b) => {

                const supplierA = a.supplierName || "";
                const supplierB = b.supplierName || "";

                if (supplierA === "" && supplierB === "") return 0;
                if (supplierA === "") return 1;
                if (supplierB === "") return -1;

                return supplierA.localeCompare(supplierB);
            });

        for (const item of sortedExportItems) {

            rows.push([
                item.supplierName || "No Supplier",
                item.materialID,
                item.materialName || "",
                item.uom || "",
                item.currentStock,
                item.requiredQuantity,
                item.pendingQuantity,
                item.orderedQuantity > 0 ? item.orderedQuantity : "",
                item.orderByDate || "",
                item.timingStatus || ""
            ]);
        }

        downloadCSV(`${projection.id}_export.csv`, rows);
    }
    catch (error) {

        console.error(error);

        alert("Could not export this Projection.");
    }
}


// ============================================================
// PROJECTIONS - DELETE
// ============================================================
// Scraps the plan outright. Procurement Orders already placed from
// it are not touched (see the backend) - only the Projection itself
// disappears from the active list.

async function deleteProjectionConfirm(id) {

    const message =
        document.getElementById(
            "projection-detail-message"
        );

    const confirmed = await showConfirmDialog(
        `Cancel Projection ${id}? It moves to Archived Projections as ` +
        "Cancelled and stops counting as demand. " +
        "Procurement Orders already placed from it will not be touched."
    );

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                "/api/projections/delete",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({ id: id })
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        await showProjections();
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROJECTIONS - REGISTER ORDER FOR SELECTED MATERIALS (ONE PRC)
// ============================================================
// Every checked material is registered together under one shared
// "PRC-######" number and one Order Date - one action, one
// Procurement Order, instead of a separate PRC per material.

// ============================================================
// PROJECTION DETAIL - REGISTER ORDER, TODAY (quick convenience)
// ============================================================
// Same one-click convenience as "Confirm Today"/"Receipt Today":
// fills the Order Date with today and reuses the normal registration
// path so validation/behavior stays identical.

function registerSelectedOrdersToday(projectionID, productID, warehouseID) {

    const dateInput =
        document.getElementById(
            "projdet-order-date"
        );

    if (!dateInput) {
        return;
    }

    const today =
        new Date().toISOString().slice(0, 10);

    dateInput.value = today;

    registerSelectedOrders(projectionID, productID, warehouseID);
}

async function registerSelectedOrders(projectionID, productID, warehouseID) {

    const message =
        document.getElementById(
            "projection-detail-message"
        );

    const orderDate =
        document.getElementById(
            "projdet-order-date"
        ).value;

    if (!orderDate) {

        message.textContent =
            "Please enter an Order Date.";

        return;
    }

    const checkboxes =
        document.querySelectorAll(
            '[id^="projdet-"][id$="-select"]'
        );

    const lines = [];

    for (const checkbox of checkboxes) {

        if (!checkbox.checked) {
            continue;
        }

        const materialID =
            checkbox.id.slice(
                "projdet-".length,
                checkbox.id.length - "-select".length
            );

        const qtyInput =
            document.getElementById(
                "projdet-" + materialID + "-qty"
            );

        const quantity =
            qtyInput ? qtyInput.value : "";

        if (!quantity ||
            Number(quantity) <= 0) {

            message.textContent =
                `Quantity to Order must be greater than zero for material ${materialID}.`;

            return;
        }

        lines.push({
            materialID: materialID,
            orderedQuantity: Number(quantity)
        });
    }

    if (lines.length === 0) {

        message.textContent =
            "Select at least one material to order.";

        return;
    }


    const payload = {

        productID: productID,

        warehouseID: Number(warehouseID),

        orderDate: orderDate,

        comment: "",

        projectionID: projectionID,

        lines: lines
    };


    try {

        const response =
            await fetch(
                "/api/procurement/create-batch",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(payload)
                }
            );

        const responseText =
            await response.text();

        if (response.ok) {

            message.textContent = "";

            // Refresh the whole batch so "Pending" for every line
            // reflects the new order right away.

            await openProjectionDetail(
                projectionID);
        }
        else {

            message.textContent =
                "Error: " + responseText;
        }
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROCUREMENT ORDERS - LIST (Open Orders / Confirmed Orders)
// ============================================================
// The active (not yet Completed) orders are split into two views by
// their combined status: Open Orders are still waiting on supplier
// confirmation (status "Ordered"), Confirmed Orders are confirmed
// and now just waiting on goods receipt ("Confirmed" or "Partially
// Received"). Both share the same fetch/group/render pipeline below,
// distinguished only by "stage" - kept in currentProcurementStage so
// every refresh call (after confirming, receiving, etc.) redraws
// whichever of the two views is actually open, without every call
// site needing to pass it explicitly.

let currentProcurementStage = "open";

async function showProcurementOrders() {

    currentProcurementStage = "open";

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Open Orders...</p>
    `;

    await displayProcurementOrders();
}

async function showConfirmedOrders() {

    currentProcurementStage = "confirmed";

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Confirmed Orders...</p>
    `;

    await displayProcurementOrders();
}

async function displayProcurementOrders() {

    const stage =
        currentProcurementStage;

    const content =
        document.getElementById(
            "procurement-content"
        );

    try {

        const response =
            await fetch("/api/procurement");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        // Completed orders move to the Archived Orders view instead
        // of staying in either active list.

        const lines =
            (data.orders || []).filter(
                order => order.status !== "Completed" &&
                         order.status !== "Closed (Incomplete)" &&
                         order.status !== "Cancelled"
            );

        const groups =
            groupProcurementLines(lines)
                .filter(group => {

                    const status =
                        combineProcurementStatus(group.lines);

                    return stage === "open"
                        ? status === "Ordered"
                        : (status === "Confirmed" ||
                           status === "Partially Received");
                });

        if (groups.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    ${stage === "open"
                        ? `No Open Orders. Orders waiting on
                           supplier confirmation appear here.`
                        : `No Confirmed Orders. Orders confirmed by
                           the supplier and waiting on goods receipt
                           appear here.`}
                </div>
            `;

            return;
        }

        content.innerHTML =
            renderProcurementGroupedList(groups, stage);

        // Each Open Order card renders its own "Add Material"
        // combobox (see renderProcurementCard) - the markup is in the
        // DOM already (cards start collapsed, not unrendered), so the
        // material cache just needs to be loaded into each one now.

        if (stage === "open") {

            for (const group of groups) {
                await initMaterialCombobox(`${group.id}-addmat`, "");
            }
        }
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// ============================================================
// NEW PROCUREMENT ORDER - FORM (from scratch, not tied to a
// Projection - one or more materials under one shared "PRC-######")
// ============================================================

function showNewProcurementOrder() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                New Procurement Order
            </h2>

            <label>
                Product (optional - for reference only)
            </label>

            ${productComboboxHtml("neworder", "Search by ID or name (optional)...")}

            <label>
                Warehouse
            </label>

            <select id="neworder-warehouse-id">
                <option value="">Loading warehouses...</option>
            </select>

            <label>
                Order Date
            </label>

            <input
                type="date"
                id="neworder-order-date"
            >

            <label>
                Comment (optional)
            </label>

            <input
                type="text"
                id="neworder-comment"
                placeholder="Enter a comment"
            >

            <label>
                Materials
            </label>

            <div id="neworder-lines">

                ${neworderRowHtml(0)}

            </div>

            <button
                type="button"
                onclick="addProcurementOrderRow()">

                + Add Material

            </button>

            <div class="form-actions">

                <button
                    onclick="submitNewProcurementOrder()">

                    Create Order

                </button>

            </div>

            <div id="neworder-message">
            </div>

        </div>
    `;

    neworderLineCounter = 1;

    populateWarehouseSelect(
        "neworder-warehouse-id",
        "",
        {
            placeholder: "Select a Warehouse",
            showID: true,
            autoSelectSingle: true
        });

    initProductCombobox("neworder");

    initMaterialCombobox("neworder-line-0");
}

// ============================================================
// NEW PROCUREMENT ORDER - MATERIAL ROW MARKUP (combobox-based)
// ============================================================

let neworderLineCounter = 0;

function neworderRowHtml(rowIndex) {

    const prefix =
        `neworder-line-${rowIndex}`;

    return `
        <div class="bom-row" id="${prefix}-row">

            ${materialComboboxHtml(prefix, "Search by ID or name...")}

            <input
                type="number"
                class="neworder-quantity"
                min="0.001"
                step="any"
                placeholder="Quantity"
            >

            <button
                type="button"
                onclick="document.getElementById('${prefix}-row').remove()">

                Remove

            </button>

        </div>
    `;
}

// ============================================================
// NEW PROCUREMENT ORDER - ADD MATERIAL ROW
// ============================================================

function addProcurementOrderRow() {

    const container =
        document.getElementById(
            "neworder-lines"
        );

    const rowIndex =
        neworderLineCounter++;

    const wrapper =
        document.createElement("div");

    wrapper.innerHTML =
        neworderRowHtml(rowIndex).trim();

    container.appendChild(
        wrapper.firstElementChild
    );

    initMaterialCombobox(
        `neworder-line-${rowIndex}`
    );
}

// ============================================================
// NEW PROCUREMENT ORDER - SUBMIT
// ============================================================
// Creates a standalone Procurement Order (or several material lines
// sharing one "PRC-######"), with no Projection attached - straight
// from Procurement Orders, for material that just needs restocking.

async function submitNewProcurementOrder() {

    const message =
        document.getElementById(
            "neworder-message"
        );

    const productID =
        document.getElementById(
            "neworder-product-id"
        ).value.trim();

    const warehouseID =
        document.getElementById(
            "neworder-warehouse-id"
        ).value;

    const orderDate =
        document.getElementById(
            "neworder-order-date"
        ).value;

    const comment =
        document.getElementById(
            "neworder-comment"
        ).value.trim();


    if (!warehouseID) {

        message.textContent =
            "Please select a Warehouse.";

        return;
    }

    if (!orderDate) {

        message.textContent =
            "Please enter an Order Date.";

        return;
    }


    const materialIDPattern =
        /^([0-9]{3}-[0-9]{6}|[0-9]{6}-00)$/;

    const rows =
        document.querySelectorAll(
            "#neworder-lines .bom-row"
        );

    const lines = [];

    for (const row of rows) {

        const materialIDInput =
            row.querySelector(
                'input[type="hidden"][id$="-material-id"]'
            );

        const materialID =
            materialIDInput ?
            materialIDInput.value.trim() : "";

        const quantity =
            Number(
                row.querySelector(
                    ".neworder-quantity"
                ).value
            );

        if (!materialID && !quantity) {
            continue;
        }

        if (!materialIDPattern.test(materialID)) {

            message.textContent =
                `Invalid Material ID: ${materialID || "(empty)"}.`;

            return;
        }

        if (!quantity || quantity <= 0) {

            message.textContent =
                `Quantity must be greater than zero for material ${materialID}.`;

            return;
        }

        lines.push({
            materialID: materialID,
            orderedQuantity: quantity
        });
    }

    if (lines.length === 0) {

        message.textContent =
            "Add at least one material with a quantity.";

        return;
    }


    const payload = {

        productID: productID,

        warehouseID: Number(warehouseID),

        orderDate: orderDate,

        comment: comment,

        projectionID: "",

        lines: lines
    };


    try {

        const response =
            await fetch(
                "/api/procurement/create-batch",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify(payload)
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        await showProcurementOrders();
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// PROCUREMENT ORDERS - ESTIMATED RECEIPT DATE
// ============================================================
// Saves the optional Estimated Receipt of a whole order (one date
// per "PRC-######") the moment the date field changes (empty =
// remove it), then updates the "ETA" shown in the card's header
// without re-drawing the list (so open cards stay open).

async function saveEstimatedReceipt(orderID) {

    const input =
        document.getElementById(`${orderID}-eta`);

    const message =
        document.getElementById(`${orderID}-eta-message`);

    if (!input || !message) {
        return;
    }

    message.classList.remove("procurement-eta-error");

    try {

        const response =
            await fetch(
                "/api/procurement/estimated-date",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({
                        id: orderID,
                        estimatedReceiptDate: input.value
                    })
                }
            );

        if (!response.ok) {

            // Put back the last saved value so the screen never
            // shows a date that was not actually stored.

            input.value =
                input.dataset.saved || "";

            message.textContent =
                await response.text();

            message.classList.add("procurement-eta-error");

            return;
        }

        input.dataset.saved = input.value;

        message.textContent =
            input.value ? "Saved" : "Removed";

        const summary =
            document.getElementById(`order-${orderID}-eta-summary`);

        if (summary) {

            summary.hidden = !input.value;

            summary.textContent =
                input.value ? "ETA " + input.value : "";
        }

        setTimeout(() => {

            if (message.textContent === "Saved" ||
                message.textContent === "Removed") {

                message.textContent = "";
            }

        }, 2000);
    }
    catch (error) {

        console.error(error);

        input.value =
            input.dataset.saved || "";

        message.textContent =
            "Could not connect to the server.";

        message.classList.add("procurement-eta-error");
    }
}

// ============================================================
// PROCUREMENT ORDERS - ARCHIVE (Completed orders only)
// ============================================================

async function showArchivedOrders() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    content.innerHTML = `
        <p>Loading Archived Orders...</p>
    `;

    await displayArchivedOrders();
}

async function displayArchivedOrders() {

    const content =
        document.getElementById(
            "procurement-content"
        );

    try {

        const response =
            await fetch("/api/procurement");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const lines =
            (data.orders || []).filter(
                order => order.status === "Completed" ||
                         order.status === "Closed (Incomplete)" ||
                         order.status === "Cancelled"
            );

        if (lines.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    No Archived (Completed) Procurement Orders yet.
                </div>
            `;

            return;
        }

        content.innerHTML =
            renderProcurementGroupedList(
                groupProcurementLines(lines),
                "archived");
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// ============================================================
// PROCUREMENT ORDERS - GROUP LINES INTO ORDERS, THEN BY SUPPLIER
// ============================================================
// One "PRC-######" id can now cover several materials (lines) -
// one action, one Procurement Order. Lines sharing an id are
// grouped into a single card; groups are then bucketed by Supplier
// (taken from the group's first line) the same way single-material
// orders were before.

function groupProcurementLines(lines) {

    const groups = [];
    const indexByID = {};

    for (const line of lines) {

        if (!(line.id in indexByID)) {

            indexByID[line.id] = groups.length;

            groups.push({
                id: line.id,
                lines: []
            });
        }

        groups[indexByID[line.id]].lines.push(line);
    }

    return groups;
}

function renderProcurementGroupedList(groups, cardMode) {

    const groupsBySupplier = {};

    for (const group of groups) {

        const supplierName =
            group.lines[0].supplierName || "No Supplier";

        if (!groupsBySupplier[supplierName]) {
            groupsBySupplier[supplierName] = [];
        }

        groupsBySupplier[supplierName].push(group);
    }

    const supplierNames =
        Object.keys(groupsBySupplier).sort((a, b) => {

            if (a === "No Supplier") return 1;
            if (b === "No Supplier") return -1;

            return a.localeCompare(b);
        });


    let html = "";

    for (const supplierName of supplierNames) {

        html += `
            <h2 class="supplier-group-heading">
                ${escapeHtml(supplierName)}
            </h2>

            <div class="procurement-list">
        `;

        for (const group of groupsBySupplier[supplierName]) {
            html += renderProcurementCard(group, cardMode);
        }

        html += `</div>`;
    }

    return html;
}

// ============================================================
// PROCUREMENT ORDERS - STATUS BADGE CLASS / COMBINING
// ============================================================

function procurementStatusClass(status) {

    if (status === "Cancelled") {
        return "status-cancelled";
    }

    if (status === "Closed (Incomplete)") {
        return "status-closed";
    }

    if (status === "Completed") {
        return "status-completed";
    }

    if (status === "Partially Received") {
        return "status-partial";
    }

    if (status === "Confirmed") {
        return "status-confirmed";
    }

    return "status-ordered";
}

// One combined status for a whole order (several material lines):
// "Closed (Incomplete)" wins if any line was closed that way (the
// order as a whole is not cleanly done); otherwise "Partially
// Received" wins whenever any line is (something has already
// arrived, but the order isn't done); otherwise the least advanced
// line's status represents the whole order, since that is what still
// needs attention.

function combineProcurementStatus(lines) {

    const stages =
        ["Ordered", "Confirmed", "Partially Received", "Completed"];

    if (lines.some(line => line.status === "Cancelled")) {
        return "Cancelled";
    }

    if (lines.some(line => line.status === "Closed (Incomplete)")) {
        return "Closed (Incomplete)";
    }

    if (lines.some(line => line.status === "Partially Received")) {
        return "Partially Received";
    }

    let earliestIndex = stages.length - 1;

    for (const line of lines) {

        const index =
            stages.indexOf(line.status);

        if (index !== -1 && index < earliestIndex) {
            earliestIndex = index;
        }
    }

    return stages[earliestIndex];
}

// ============================================================
// PROCUREMENT ORDERS - DOCUMENT ROWS MARKUP (Order Confirmation /
// Lieferschein), shared by the initial card render and by the
// in-place refresh after an upload (see uploadProcurementDocument) -
// factored out so uploading a document never has to re-render the
// whole card (which would collapse it and throw away whatever the
// operator already typed into Confirmation Date / Qty).
// ============================================================

function renderProcurementDocumentRows(
    id,
    orderConfirmationPath,
    lieferscheinPaths,
    canAttachLieferschein) {

    return `

        <div class="procurement-document-row">

            <span>Order Confirmation:</span>

            ${orderConfirmationPath
                ? `<a href="/api/procurement/${encodeURIComponent(id)}/documents/order-confirmation/file" target="_blank">View file</a>
                   <button type="button" class="procurement-document-remove"
                       title="Remove this attachment"
                       onclick="removeProcurementDocument('${id}', 'order-confirmation', 0)">&times;</button>`
                : `<em>Not attached</em>`}

            <label class="procurement-document-upload">
                ${orderConfirmationPath ? "Replace" : "Upload"}
                <input
                    type="file"
                    accept="application/pdf,image/jpeg,image/png"
                    class="hidden"
                    onchange="uploadProcurementDocument('${id}', 'order-confirmation', this)">
            </label>

        </div>

        <div class="procurement-document-row">

            <span>Lieferschein:</span>

            ${canAttachLieferschein
                ? `
                    ${lieferscheinPaths.length
                        ? lieferscheinPaths
                            .map((path, index) => `<a href="/api/procurement/${encodeURIComponent(id)}/documents/lieferschein/${index + 1}/file" target="_blank">View file ${index + 1}</a><button type="button" class="procurement-document-remove" title="Remove this attachment" onclick="removeProcurementDocument('${id}', 'lieferschein', ${index + 1})">&times;</button>`)
                            .join(" ")
                        : `<em>Not attached</em>`}

                    <label class="procurement-document-upload">
                        + Add Delivery Note
                        <input
                            type="file"
                            accept="application/pdf,image/jpeg,image/png"
                            class="hidden"
                            onchange="uploadProcurementDocument('${id}', 'lieferschein', this)">
                    </label>
                `
                : `<em>Available once the order is confirmed</em>`}

        </div>
    `;
}

// ============================================================
// PROCUREMENT ORDERS - CARD MARKUP (one order, 1+ material lines)
// ============================================================

function renderProcurementCard(group, cardMode = "open") {

    const id = group.id;
    const lines = group.lines;

    const combinedStatus =
        combineProcurementStatus(lines);

    const totalOrdered =
        roundQty3(lines.reduce((sum, line) => sum + line.orderedQuantity, 0));

    const totalReceived =
        roundQty3(lines.reduce((sum, line) => sum + line.totalReceivedQuantity, 0));

    // Material IDs open the material pop-up (without toggling the card)
    const materialLinkHtml = materialID => `
        <a href="#" class="material-popup-link"
            title="Show material information"
            onclick="event.preventDefault(); event.stopPropagation(); openMaterialFromQuotation('${escapeHtml(materialID)}');">${escapeHtml(materialID)}</a>`.trim();

    // The collapsed header is plain text (no links); the material IDs
    // inside the expanded order are the links
    const materialsLabelHtml =
        lines.length === 1
        ? `${escapeHtml(lines[0].materialID)} ${escapeHtml(lines[0].materialName || "")}` +
            (lines[0].supplierPartNumber ?
                `  \u00b7  Supplier P/N: ${escapeHtml(lines[0].supplierPartNumber)}` : "")
        : lines.map(line => escapeHtml(line.materialID)).join(", ");

    const warehouseID =
        lines[0].warehouseID;

    // Optional "Estimated Receipt": ONE date for the whole order
    // (when the supplier is expected to deliver), applied by the
    // server to every line still waiting for goods. Saved on its own
    // as soon as it changes (clearing the field removes it). A
    // single-material order shows it in the material's row; an order
    // with several materials shows it once above them. It is also
    // shown as "ETA" in the collapsed header.

    const waitingLines =
        lines.filter(line => line.pendingQuantity > 0);

    const etaEditable =
        (cardMode === "open" || cardMode === "confirmed") &&
        waitingLines.length > 0;

    const earliestEta =
        etaEditable
        ? (waitingLines.find(line => line.estimatedReceiptDate) || {})
            .estimatedReceiptDate || ""
        : "";

    const latestOrderDate =
        lines.map(line => line.orderDate || "").sort().pop() || "";

    const etaReadOnly =
        currentUser && currentUser.role === "viewer";

    const estimatedReceiptFieldHtml =
        etaEditable
        ? `
            <div class="detail-field procurement-eta-field">
                <div class="procurement-eta-inline">
                    <span class="detail-label">Estimated Delivery</span>
                    <input
                        type="date"
                        class="procurement-eta-input"
                        id="${id}-eta"
                        data-saved="${escapeHtml(earliestEta)}"
                        value="${escapeHtml(earliestEta)}"
                        min="${escapeHtml(latestOrderDate)}"
                        ${etaReadOnly ? "disabled" : ""}
                        onchange="saveEstimatedReceipt('${id}')"
                    >
                    <span
                        class="procurement-eta-message"
                        id="${id}-eta-message">
                    </span>
                </div>
            </div>
        `
        : "";

    const sharedEtaRowHtml =
        lines.length > 1 && etaEditable
        ? `<div class="procurement-eta-row">${estimatedReceiptFieldHtml}</div>`
        : "";

    // One shared Confirmation Date for the whole order, but each
    // material line keeps its own Confirmed Quantity - use whichever
    // line already has a date saved (they're kept in sync by
    // saveGroupConfirmation).

    const sharedConfirmationDate =
        (lines.find(line => line.confirmationDate) || {})
            .confirmationDate || "";


    // One block per material: Order Date / Ordered Qty /
    // Received-Pending, plus (mode-dependent) either its own Qty
    // field right there in the same row (Open Orders - so the field
    // to confirm sits next to that material's own info instead of a
    // separate section further down), or its own Add Receipt fields
    // directly underneath (Confirmed Orders), plus its receipt
    // history if it has any. Only labeled with the material when an
    // order covers more than one.

    let materialRowsHtml = "";

    for (const line of lines) {

        const materialHeaderHtml =
            true
            ? `
                <div class="procurement-line-header">
                    <strong>${materialLinkHtml(line.materialID)}</strong>
                    ${escapeHtml(line.materialName || "")}
                    ${line.supplierPartNumber ?
                        `<span class="procurement-supplier-pn">
                            &nbsp;&middot;&nbsp; Supplier P/N:
                            <strong>${escapeHtml(line.supplierPartNumber)}</strong>
                        </span>` : ""}
                </div>
            `
            : "";

        const confirmQtyFieldHtml =
            cardMode === "open"
            ? `
                <div class="detail-field">
                    <div class="detail-label">Qty</div>
                    <input
                        type="number"
                        min="0"
                        step="any"
                        class="procurement-qty-input"
                        id="${id}-${line.materialID}-confirm-qty"
                        placeholder="Qty (0 = delete)"
                        value="${line.confirmedQuantity > 0 ? line.confirmedQuantity : ""}"
                        data-ordered-qty="${line.orderedQuantity}"
                    >
                </div>
            `
            : "";

        // Confirmed orders: the confirmed quantity can still be changed
        // (e.g. the supplier delivers more than confirmed) - only
        // shown after the order's single "Edit" button was clicked.
        const changeQtyFieldHtml =
            (cardMode === "confirmed" &&
             line.status !== "Cancelled" &&
             line.status !== "Closed (Incomplete)")
            ? `
                <div class="detail-field order-edit-only">
                    <div class="detail-label">Confirmed Qty</div>
                    <input
                        type="number"
                        min="0.001"
                        step="any"
                        class="procurement-qty-input order-edit-qty"
                        data-material="${escapeHtml(line.materialID)}"
                        data-original="${line.confirmedQuantity > 0 ? line.confirmedQuantity : line.orderedQuantity}"
                        value="${line.confirmedQuantity > 0 ? line.confirmedQuantity : line.orderedQuantity}"
                    >
                </div>
            `
            : "";

        let addReceiptHtml = "";

        if (cardMode === "confirmed" && line.pendingQuantity > 0) {

            addReceiptHtml = `

                <div class="procurement-section">

                    <div class="detail-label">
                        Add Receipt
                    </div>

                    <div class="procurement-inline-form">

                        <input
                            type="date"
                            id="${id}-${line.materialID}-receipt-date"
                        >

                        <input
                            type="number"
                            min="0.001"
                            step="any"
                            max="${line.pendingQuantity}"
                            class="procurement-qty-input"
                            id="${id}-${line.materialID}-receipt-qty"
                            placeholder="Receive"
                        >

                        <input
                            type="text"
                            id="${id}-${line.materialID}-receipt-comment"
                            placeholder="Comment (optional)"
                        >

                    </div>

                </div>
            `;
        }

        let receiptsHtml = "";

        if (cardMode !== "open" &&
            line.receipts && line.receipts.length > 0) {

            receiptsHtml = `
                <div class="procurement-receipts">

                    <div class="detail-label">
                        Receipt History
                    </div>

                    ${line.receipts.map((receipt, receiptIdx) => `
                        <div class="procurement-receipt-row">
                            ${escapeHtml(receipt.receiptDate)}
                            &mdash;
                            ${currentUser && currentUser.role === "admin" ? `
                                <span class="order-view-only">${receipt.receivedQuantity}</span>
                                <input
                                    type="number"
                                    min="0"
                                    step="any"
                                    class="procurement-qty-input order-edit-only receipt-edit-qty"
                                    data-material="${escapeHtml(line.materialID)}"
                                    data-index="${receiptIdx}"
                                    data-date="${escapeHtml(receipt.receiptDate)}"
                                    data-original="${receipt.receivedQuantity}"
                                    value="${receipt.receivedQuantity}"
                                    title="Admin: reduce this receipt (0 = remove it)"
                                >
                            ` : receipt.receivedQuantity}
                            ${receipt.comment
                                ? "(" + escapeHtml(receipt.comment) + ")"
                                : ""}
                        </div>
                    `).join("")}

                </div>
            `;
        }

        // Received/Pending is meaningless before the order is even
        // confirmed by the supplier - at this stage (Open Orders)
        // this is still just the Order Confirmation, nothing has
        // been received or is "pending" yet.

        const receivedPendingFieldHtml =
            cardMode === "open"
            ? ""
            : `
                <div class="detail-field">
                    <div class="detail-label">Received / Pending</div>
                    <div class="detail-value">${line.totalReceivedQuantity} / ${line.pendingQuantity}</div>
                </div>
            `;

        materialRowsHtml += `

            <div class="procurement-line">

                ${materialHeaderHtml}

                <div class="material-detail-grid procurement-grid procurement-grid-compact">

                    <div class="detail-field">
                        <div class="detail-label">Order Date</div>
                        <div class="detail-value">${escapeHtml(line.orderDate)}</div>
                    </div>

                    <div class="detail-field">
                        <div class="detail-label">Ordered Qty</div>
                        <div class="detail-value">${line.orderedQuantity}</div>
                    </div>

                    ${receivedPendingFieldHtml}

                    ${confirmQtyFieldHtml}

                    ${changeQtyFieldHtml}

                    ${lines.length === 1 ? estimatedReceiptFieldHtml : ""}

                </div>

                ${addReceiptHtml}

                ${receiptsHtml}

            </div>
        `;
    }


    // Add Material (Open Orders only): sometimes another Material for
    // the same Supplier turns out to be needed only after this
    // "PRC-######" was already placed - this appends it as one more
    // line on the same Order (same Warehouse/Order Date) instead of
    // forcing a whole separate Order for it. Only offered while the
    // Order is still fully unconfirmed - see the backend route.

    let addMaterialHtml = "";

    if (cardMode === "open") {

        const addMaterialPrefix =
            `${id}-addmat`;

        addMaterialHtml = `

            <div class="procurement-section">

                <div class="detail-label">
                    Add Material
                </div>

                <div class="procurement-inline-form">

                    ${materialComboboxHtml(
                        addMaterialPrefix,
                        "Search by ID or name...")}

                    <input
                        type="number"
                        min="0.001"
                        step="any"
                        class="procurement-qty-input"
                        id="${addMaterialPrefix}-qty"
                        placeholder="Qty"
                    >

                    <button
                        type="button"
                        onclick="addMaterialToOrder('${id}')">

                        Add Material

                    </button>

                </div>

                <div id="${addMaterialPrefix}-message" class="procurement-card-message">
                </div>

            </div>
        `;
    }


    // Confirmation (Open Orders only): one shared Order Date and pair
    // of actions for the whole order plus the resulting status badge -
    // the per-material Qty fields now live in each material's own row
    // above, not here.

    let confirmationHtml = "";

    if (cardMode === "open") {

        confirmationHtml = `

            <div class="procurement-section">

                <div class="detail-label">
                    Confirmation
                </div>

                <div class="procurement-inline-form">

                    <input
                        type="date"
                        id="${id}-confirm-date"
                        value="${escapeHtml(sharedConfirmationDate)}"
                    >

                    <button
                        type="button"
                        onclick="saveGroupConfirmation('${id}')">

                        Confirm

                    </button>

                    <button
                        type="button"
                        class="button-today"
                        onclick="quickConfirmGroup('${id}')">

                        Confirm Today

                    </button>

                    <button
                        type="button"
                        class="button-danger"
                        onclick="cancelOrderGroup('${id}')">

                        Cancel

                    </button>

                    <span class="status-badge procurement-confirm-status ${procurementStatusClass(combinedStatus)}">
                        ${escapeHtml(combinedStatus)}
                    </span>

                </div>

            </div>
        `;
    }


    // Material Receipt (Confirmed Orders only): one shared button
    // under every material's own Add Receipt fields - one click
    // receives whichever materials were given a quantity above.

    let receiptButtonHtml = "";

    if (cardMode === "confirmed" &&
        lines.some(line => line.pendingQuantity > 0)) {

        receiptButtonHtml = `

            <div class="procurement-inline-form">

                <button
                    type="button"
                    class="button-danger"
                    onclick="closeOrderGroup('${id}')">

                    Close Order

                </button>

                <button
                    type="button"
                    onclick="saveGroupReceipts('${id}')">

                    Material Receipt

                </button>

                <button
                    type="button"
                    class="button-today"
                    onclick="quickReceiptGroup('${id}')">

                    Receipt Today

                </button>

            </div>
        `;
    }

    // Single "Edit" button for the whole order: confirmed orders can
    // change their Confirmed Qty (Editor / Admin); received quantities
    // can be corrected afterwards by an Admin only.

    const isAdminUser =
        currentUser && currentUser.role === "admin";

    const canEditOrder =
        cardMode === "confirmed"
            ? true
            : (cardMode === "archived" && isAdminUser &&
               lines.some(line =>
                   line.receipts && line.receipts.length > 0));

    const editBarHtml = canEditOrder ? `

        <div class="order-edit-bar">

            <button
                type="button"
                class="order-edit-btn order-view-only"
                onclick="modifyOrderStart('${id}')">

                Edit Order

            </button>

            <button
                type="button"
                class="order-edit-btn order-edit-only"
                onclick="modifyOrderSave('${id}', '${cardMode}')">

                Save changes

            </button>

            <button
                type="button"
                class="order-edit-btn order-edit-only"
                onclick="modifyOrderCancel('${id}')">

                Cancel

            </button>

        </div>
    ` : "";

    // Documents (Order Confirmation / Lieferschein): shared across
    // every line of the whole "PRC-######" - see
    // ProcurementManager::setOrderConfirmationPath(). Order
    // Confirmation is a single file that a new upload replaces;
    // Lieferschein can be several files (one per shipment, for a
    // partial delivery), so a new upload always adds another one
    // instead of replacing. Lieferschein only makes sense once the
    // supplier has actually confirmed the order.

    const orderConfirmationPath =
        lines[0].orderConfirmationPath || "";

    const lieferscheinPaths =
        lines[0].lieferscheinPaths || [];

    const canAttachLieferschein =
        combinedStatus !== "Ordered" &&
        combinedStatus !== "Cancelled";

    const documentsHtml = `

        <div class="procurement-section">

            <div class="detail-label">
                Documents
            </div>

            <div id="${id}-documents-rows">
                ${renderProcurementDocumentRows(
                    id,
                    orderConfirmationPath,
                    lieferscheinPaths,
                    canAttachLieferschein)}
            </div>

            <div id="${id}-documents-message" class="procurement-card-message">
            </div>

        </div>
    `;


    return `

        <div class="procurement-card collapsed" id="order-${id}">

            <div
                class="procurement-summary"
                onclick="toggleProcurementCard('${id}')">

                <span class="procurement-summary-id">
                    ${escapeHtml(id)}
                </span>

                <span class="procurement-summary-material">
                    ${materialsLabelHtml}
                </span>

                <span class="procurement-summary-qty">
                    WH ${warehouseID}
                    &nbsp;&middot;&nbsp;
                    ${totalReceived} / ${totalOrdered}
                </span>

                <span
                    class="procurement-summary-eta"
                    id="order-${id}-eta-summary"
                    ${earliestEta ? "" : "hidden"}>
                    ${earliestEta ? "ETA " + escapeHtml(earliestEta) : ""}
                </span>

                <span class="status-badge ${procurementStatusClass(combinedStatus)}">
                    ${escapeHtml(combinedStatus)}
                </span>

                <span class="procurement-summary-caret">
                    &#9656;
                </span>

            </div>

            <div class="procurement-body" id="order-${id}-body" hidden>

                ${sharedEtaRowHtml}

                ${materialRowsHtml}

                ${addMaterialHtml}

                ${confirmationHtml}

                ${receiptButtonHtml}

                ${documentsHtml}

                <div id="${id}-message" class="procurement-card-message">
                </div>

                ${editBarHtml}

            </div>

        </div>
    `;
}

// ============================================================
// PROCUREMENT ORDERS - UPLOAD DOCUMENT (Order Confirmation /
// Lieferschein)
// ============================================================
// kind is "order-confirmation" or "lieferschein", matching the
// backend routes - one of each per whole "PRC-######", shared across
// every material line, saved under the Documents Folder configured
// in Data Management Settings.

// Takes an attachment off the order (the file stays in the Documents
// Folder). index: 1-based number of the Lieferschein.
async function removeProcurementDocument(id, kind, index) {

    const message =
        document.getElementById(`${id}-documents-message`);

    const confirmed =
        await showConfirmDialog(
            "Remove this attachment from the order? The file itself " +
            "stays in the Documents Folder.");

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                `/api/procurement/${encodeURIComponent(id)}/documents/remove`,
                {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ kind: kind, index: index })
                });

        if (!response.ok) {

            if (message) {
                message.textContent = await response.text();
            }

            return;
        }

        const ordersResponse = await fetch("/api/procurement");

        if (ordersResponse.ok) {

            const ordersData = await ordersResponse.json();

            const orderLines =
                (ordersData.orders || [])
                    .filter(order => order.id === id);

            const rowsContainer =
                document.getElementById(`${id}-documents-rows`);

            if (orderLines.length > 0 && rowsContainer) {

                const combinedStatus =
                    combineProcurementStatus(orderLines);

                rowsContainer.innerHTML =
                    renderProcurementDocumentRows(
                        id,
                        orderLines[0].orderConfirmationPath || "",
                        orderLines[0].lieferscheinPaths || [],
                        combinedStatus !== "Ordered" &&
                            combinedStatus !== "Cancelled");
            }
        }

        if (message) {
            message.textContent = "";
        }
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent = "Could not connect to the server.";
        }
    }
}

// A photo (JPG / PNG) is shrunk to at most 2000 px on its long side and
// saved as JPEG before it is sent (phone photos are several MB); a PDF
// goes up unchanged.
async function readDocumentForUpload(file) {

    if (!file.type.startsWith("image/")) {
        return readFileAsBase64(file);
    }

    const url = URL.createObjectURL(file);

    try {

        const image = await new Promise((resolve, reject) => {
            const img = new Image();
            img.onload = () => resolve(img);
            img.onerror = () => reject(new Error("Could not read the photo."));
            img.src = url;
        });

        const scale =
            Math.min(1, 2000 / Math.max(image.width, image.height));

        const canvas = document.createElement("canvas");

        canvas.width = Math.round(image.width * scale);
        canvas.height = Math.round(image.height * scale);

        const context = canvas.getContext("2d");

        context.fillStyle = "#ffffff";
        context.fillRect(0, 0, canvas.width, canvas.height);
        context.drawImage(image, 0, 0, canvas.width, canvas.height);

        return canvas.toDataURL("image/jpeg", 0.85);
    }
    finally {
        URL.revokeObjectURL(url);
    }
}

async function uploadProcurementDocument(id, kind, inputEl) {

    const file =
        inputEl.files[0];

    if (!file) {
        return;
    }

    const message =
        document.getElementById(
            `${id}-documents-message`
        );

    if (message) {
        message.textContent = "Uploading...";
    }

    try {

        const fileData =
            await readDocumentForUpload(file);

        const response =
            await fetch(
                `/api/procurement/${encodeURIComponent(id)}/documents/${kind}`,
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({
                        fileData: fileData
                    })
                }
            );

        if (!response.ok) {

            const text =
                await response.text();

            if (message) {
                message.textContent = text;
            }

            // Reset the file input so the same file can be retried.
            inputEl.value = "";

            return;
        }

        // Refresh only this card's Documents rows in place (new "View
        // PDF" link, "Replace"/"+ Add Delivery Note" label, etc.)
        // instead of re-rendering the whole Open/Confirmed Orders
        // list. A full re-render would collapse every card back down
        // and wipe out a Confirmation Date/Qty the operator already
        // started typing into THIS SAME card - forcing two separate
        // round trips (attach the file, re-open the card, then fill
        // in the rest) instead of doing everything in one.

        const ordersResponse =
            await fetch("/api/procurement");

        if (ordersResponse.ok) {

            const ordersData =
                await ordersResponse.json();

            const orderLines =
                (ordersData.orders || [])
                    .filter(order => order.id === id);

            if (orderLines.length > 0) {

                const combinedStatus =
                    combineProcurementStatus(orderLines);

                const rowsContainer =
                    document.getElementById(
                        `${id}-documents-rows`
                    );

                if (rowsContainer) {

                    rowsContainer.innerHTML =
                        renderProcurementDocumentRows(
                            id,
                            orderLines[0].orderConfirmationPath || "",
                            orderLines[0].lieferscheinPaths || [],
                            combinedStatus !== "Ordered" &&
                                combinedStatus !== "Cancelled");
                }
            }
        }

        if (message) {
            message.textContent = "";
        }
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent = "Could not connect to the server.";
        }
    }
}

// ============================================================
// PROCUREMENT ORDERS - TOGGLE (expand/collapse in place)
// ============================================================

function toggleProcurementCard(id) {

    const card =
        document.getElementById("order-" + id);

    const body =
        document.getElementById("order-" + id + "-body");

    if (!card || !body) {
        return;
    }

    body.hidden = !body.hidden;

    card.classList.toggle(
        "collapsed",
        body.hidden);
}

// ============================================================
// PROCUREMENT ORDERS - QUICK CONFIRM (today, same ordered qty)
// ============================================================
// One click for the common case: supplier confirmed exactly what
// was ordered, today, for every material in this order. Fills the
// Confirmation fields and reuses the normal save path so
// validation/behavior stays identical.

function quickConfirmGroup(id) {

    const dateInput =
        document.getElementById(
            id + "-confirm-date"
        );

    if (!dateInput) {
        return;
    }

    const today =
        new Date().toISOString().split("T")[0];

    dateInput.value = today;

    const qtyInputs =
        document.querySelectorAll(
            `[id^="${id}-"][id$="-confirm-qty"]`
        );

    for (const input of qtyInputs) {
        input.value = input.dataset.orderedQty;
    }

    saveGroupConfirmation(id);
}

// ============================================================
// PROCUREMENT ORDERS - SAVE CONFIRMATION (whole order at once)
// ============================================================
// One Confirmation Date applies to every material line in this
// order, but each line keeps its own Confirmed Quantity. Saved one
// line at a time against the existing /api/procurement/confirm
// endpoint (which already deletes a line outright when its
// confirmed quantity is 0) so server-side behavior/validation stays
// exactly as it is for a single-material order.

async function saveGroupConfirmation(id) {

    const message =
        document.getElementById(
            id + "-message"
        );

    const confirmationDate =
        document.getElementById(
            id + "-confirm-date"
        ).value;

    if (!confirmationDate) {

        message.textContent =
            "Please enter a Confirmation Date.";

        return;
    }

    const qtyInputs =
        document.querySelectorAll(
            `[id^="${id}-"][id$="-confirm-qty"]`
        );

    const lineUpdates = [];

    for (const input of qtyInputs) {

        const materialID =
            input.id.slice(
                id.length + 1,
                input.id.length - "-confirm-qty".length
            );

        if (input.value === "" ||
            Number(input.value) < 0) {

            message.textContent =
                `Please enter a Confirmed Quantity for ${materialID} ` +
                `(0 deletes that material from the order).`;

            return;
        }

        lineUpdates.push({
            materialID: materialID,
            confirmedQuantity: Number(input.value)
        });
    }

    const toDelete =
        lineUpdates.filter(line => line.confirmedQuantity === 0);

    if (toDelete.length > 0) {

        const confirmed =
            await showConfirmDialog(
                "Setting the Confirmed Quantity to 0 will remove " +
                toDelete.map(line => line.materialID).join(", ") +
                ` from Procurement Order ${id}. This cannot be undone. ` +
                "Continue?"
            );

        if (!confirmed) {
            return;
        }
    }

    message.textContent = "";

    for (const line of lineUpdates) {

        try {

            const response =
                await fetch(
                    "/api/procurement/confirm",
                    {
                        method: "POST",

                        headers: {
                            "Content-Type":
                                "application/json"
                        },

                        body: JSON.stringify({
                            id: id,
                            materialID: line.materialID,
                            confirmationDate: confirmationDate,
                            confirmedQuantity: line.confirmedQuantity
                        })
                    }
                );

            if (!response.ok) {

                const responseText =
                    await response.text();

                message.textContent =
                    `Error (${line.materialID}): ` + responseText;

                await displayProcurementOrders();

                return;
            }
        }
        catch (error) {

            console.error(error);

            message.textContent =
                "Could not connect to the server.";

            return;
        }
    }

    await displayProcurementOrders();
}

// ============================================================
// PROCUREMENT ORDERS - QUICK RECEIPT (today, whatever Qty is filled)
// ============================================================
// Same one-click convenience as "Confirm Today": fills today's date
// into every material's Receipt Date field, then reuses the normal
// save path - the operator still enters each Qty by hand
// (there is no "ordered quantity" default here, since a receipt is
// often partial).

function quickReceiptGroup(id) {

    const today =
        new Date().toISOString().split("T")[0];

    const dateInputs =
        document.querySelectorAll(
            `[id^="${id}-"][id$="-receipt-date"]`
        );

    for (const input of dateInputs) {
        input.value = today;
    }

    saveGroupReceipts(id);
}

// ============================================================
// PROCUREMENT ORDERS - CLOSE ORDER (won't fully arrive)
// ============================================================
// For when the rest of an order is simply never coming (goods lost,
// discontinued, cancelled after a partial delivery...). Closes every
// still-pending material line under this order in one action -
// whatever was already received stays in inventory - and the order
// moves out of Confirmed Orders into Archived Orders, marked
// "Closed (Incomplete)".

async function closeOrderGroup(id) {

    const message =
        document.getElementById(
            `${id}-message`
        );

    const confirmed =
        await showConfirmDialog(
            `Close Procurement Order ${id}? Whatever has already ` +
            "been received stays in inventory, but the rest will no " +
            "longer count as pending and this order will move to " +
            "Archived Orders as \"Closed (Incomplete)\". This cannot " +
            "be undone."
        );

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                "/api/procurement/close",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body: JSON.stringify({ id: id })
                }
            );

        if (!response.ok) {

            const errorText =
                await response.text();

            if (message) {
                message.textContent =
                    "Error: " + errorText;
            }

            return;
        }

        await displayProcurementOrders();
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent =
                "Could not connect to the server.";
        }
    }
}

// ============================================================
// PROCUREMENT ORDERS - CANCEL ORDER (never confirmed)
// ============================================================
// For an order still sitting in Open Orders that will never be
// confirmed by the supplier. Moves it to Archived Orders as
// "Cancelled" - distinct from "Closed (Incomplete)", which is for
// an order that was confirmed and partially received.

async function cancelOrderGroup(id) {

    const message =
        document.getElementById(
            `${id}-message`
        );

    const confirmed =
        await showConfirmDialog(
            `Cancel Procurement Order ${id}? This order will move to ` +
            "Archived Orders as \"Cancelled\". This cannot be undone."
        );

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                "/api/procurement/cancel",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body: JSON.stringify({ id: id })
                }
            );

        if (!response.ok) {

            const errorText =
                await response.text();

            if (message) {
                message.textContent =
                    "Error: " + errorText;
            }

            return;
        }

        await displayProcurementOrders();
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent =
                "Could not connect to the server.";
        }
    }
}

// ============================================================
// PROCUREMENT ORDERS - ADD MATERIAL TO AN EXISTING OPEN ORDER
// ============================================================
// Lets one more Material be added straight onto an already-placed
// but still unconfirmed "PRC-######", instead of forcing a whole new
// Order for something that came up after the fact for the same
// Supplier - see the backend route for the full reasoning and the
// checks it runs (same Supplier, not yet confirmed, not already on
// this order).

async function addMaterialToOrder(id) {

    const prefix =
        `${id}-addmat`;

    const materialID =
        document.getElementById(
            `${prefix}-material-id`
        ).value.trim();

    const qtyInput =
        document.getElementById(
            `${prefix}-qty`
        );

    const orderedQuantity =
        Number(qtyInput.value);

    const message =
        document.getElementById(
            `${prefix}-message`
        );

    if (message) {
        message.textContent = "";
    }

    if (!materialID) {

        if (message) {
            message.textContent =
                "Please select a Material from the list.";
        }

        return;
    }

    if (!qtyInput.value || orderedQuantity <= 0) {

        if (message) {
            message.textContent =
                "Please enter a Quantity greater than zero.";
        }

        return;
    }

    try {

        const response =
            await fetch(
                `/api/procurement/${encodeURIComponent(id)}/add-material`,
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body: JSON.stringify({
                        materialID: materialID,
                        orderedQuantity: orderedQuantity
                    })
                }
            );

        if (!response.ok) {

            const errorText =
                await response.text();

            if (message) {
                message.textContent =
                    "Error: " + errorText;
            }

            return;
        }

        await displayProcurementOrders();
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent =
                "Could not connect to the server.";
        }
    }
}

// ============================================================
// PROCUREMENT ORDERS - SAVE RECEIPT (triggers Goods Receipt)
// ============================================================
// One click for the whole order: every material line in the Add
// Receipt block that was given a quantity is received, in turn,
// through the same /api/procurement/receive endpoint used before -
// only now the operator does not need to click a separate button
// per material.

// ============================================================
// PROCUREMENT ORDERS - EDIT (one button for the whole order)
// ============================================================
// Confirmed Qty of every material line (Editor / Admin) and, for an
// Admin, the already booked receipts (to fix e.g. a receipt saved
// twice). Every change is checked by the server and written to the
// movement record with the material and "from -> to".

function modifyOrderStart(id) {

    const card = document.getElementById(`order-${id}`);

    if (card) {
        card.classList.add("editing");
    }
}

function modifyOrderCancel(id) {

    const card = document.getElementById(`order-${id}`);

    if (!card) {
        return;
    }

    for (const input of card.querySelectorAll(
        ".order-edit-qty, .receipt-edit-qty")) {

        input.value = input.dataset.original;
    }

    card.classList.remove("editing");

    const message = document.getElementById(`${id}-message`);

    if (message) {
        message.textContent = "";
    }
}

async function modifyOrderSave(id, cardMode) {

    const card = document.getElementById(`order-${id}`);

    const message = document.getElementById(`${id}-message`);

    if (!card) {
        return;
    }

    const quantityChanges = [];

    const receiptCorrections = [];

    const summary = [];

    for (const input of card.querySelectorAll(".order-edit-qty")) {

        const original = Number(input.dataset.original);

        const value = Number(input.value);

        if (input.value === "" || !(value > 0)) {

            if (message) {
                message.textContent =
                    `Confirmed Qty of ${input.dataset.material} must be greater than zero.`;
            }

            return;
        }

        if (value !== original) {

            quantityChanges.push({
                materialID: input.dataset.material,
                confirmedQuantity: value
            });

            summary.push(
                `${input.dataset.material}: Confirmed Qty ${original} \u2192 ${value}`);
        }
    }

    for (const input of card.querySelectorAll(".receipt-edit-qty")) {

        const original = Number(input.dataset.original);

        const value = Number(input.value);

        if (input.value === "" || value < 0) {

            if (message) {
                message.textContent =
                    `Enter a valid quantity for the receipt of ${input.dataset.material}.`;
            }

            return;
        }

        if (value !== original) {

            receiptCorrections.push({
                materialID: input.dataset.material,
                receiptIndex: Number(input.dataset.index),
                quantity: value
            });

            summary.push(
                `${input.dataset.material}: receipt of ${input.dataset.date} ` +
                `${original} \u2192 ${value}` +
                ` (stock ${roundQty3(value - original)})`);
        }
    }

    if (summary.length === 0) {

        if (message) {
            message.textContent = "Nothing was changed.";
        }

        return;
    }

    const confirmed =
        await showConfirmDialog(
            `Save these changes to ${id}?\n\n` + summary.join("\n"));

    if (!confirmed) {
        return;
    }

    try {

        if (quantityChanges.length > 0) {

            const response =
                await fetch("/api/procurement/edit-quantities", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({
                        id: id,
                        changes: quantityChanges
                    })
                });

            if (!response.ok) {

                if (message) {
                    message.textContent =
                        "Error: " + await response.text();
                }

                return;
            }
        }

        if (receiptCorrections.length > 0) {

            const response =
                await fetch("/api/procurement/correct-receipts", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({
                        id: id,
                        corrections: receiptCorrections
                    })
                });

            if (!response.ok) {

                if (message) {
                    message.textContent =
                        "Error: " + await response.text();
                }

                return;
            }
        }

        if (cardMode === "archived") {
            await displayArchivedOrders();
        }
        else {
            await displayProcurementOrders();
        }
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent =
                "Could not connect to the server.";
        }
    }
}

const receiptSavesRunning = new Set();

async function saveGroupReceipts(id) {

    // One save at a time per order: the receipt is sent material by
    // material, so a second click while the first is still running
    // would receive every material twice.
    if (receiptSavesRunning.has(id)) {
        return;
    }

    receiptSavesRunning.add(id);

    try {
        await saveGroupReceiptsInner(id);
    }
    finally {
        receiptSavesRunning.delete(id);
    }
}

async function saveGroupReceiptsInner(id) {

    const message =
        document.getElementById(
            `${id}-message`
        );

    if (message) {
        message.textContent = "";
    }

    const qtySuffix =
        "-receipt-qty";

    const qtyInputs =
        document.querySelectorAll(
            `[id^="${id}-"][id$="${qtySuffix}"]`
        );

    const toSubmit = [];

    for (const qtyInput of qtyInputs) {

        const materialID =
            qtyInput.id.slice(
                id.length + 1,
                qtyInput.id.length - qtySuffix.length
            );

        const rawQty =
            qtyInput.value;

        if (!rawQty ||
            Number(rawQty) <= 0) {
            continue;
        }

        const dateInput =
            document.getElementById(
                `${id}-${materialID}-receipt-date`
            );

        const commentInput =
            document.getElementById(
                `${id}-${materialID}-receipt-comment`
            );

        if (!dateInput || !dateInput.value) {

            if (message) {
                message.textContent =
                    `Please enter a Receipt Date for ${materialID}.`;
            }

            return;
        }

        toSubmit.push({
            materialID: materialID,
            receiptDate: dateInput.value,
            receivedQuantity: Number(rawQty),
            comment: commentInput ?
                commentInput.value.trim() : ""
        });
    }

    if (toSubmit.length === 0) {

        if (message) {
            message.textContent =
                "Enter a received quantity for at least one material.";
        }

        return;
    }

    try {

        for (const entry of toSubmit) {

            const response =
                await fetch(
                    "/api/procurement/receive",
                    {
                        method: "POST",

                        headers: {
                            "Content-Type":
                                "application/json"
                        },

                        body: JSON.stringify({
                            id: id,
                            materialID: entry.materialID,
                            receiptDate: entry.receiptDate,
                            receivedQuantity: entry.receivedQuantity,
                            comment: entry.comment
                        })
                    }
                );

            if (!response.ok) {

                const errorText =
                    await response.text();

                await displayProcurementOrders();

                const refreshedMessage =
                    document.getElementById(
                        `${id}-message`
                    );

                if (refreshedMessage) {
                    refreshedMessage.textContent =
                        `Error (${entry.materialID}): ${errorText}`;
                }

                return;
            }
        }

        await displayProcurementOrders();
    }
    catch (error) {

        console.error(error);

        if (message) {
            message.textContent =
                "Could not connect to the server.";
        }
    }
}
// ============================================================
// PURCHASE MODULE
// ============================================================
// Prices already-received Procurement deliveries using landed cost
// (material cost plus customs/freight, prorated "by value" across an
// invoice's lines), keeps every price ever entered as history, and
// rolls Products up to a current cost from their BOM. Everything is
// shown in EUR; a USD invoice is converted using the exchange rate
// entered on that invoice, which is preserved even if the module's
// default rate is later changed.

function formatMoney(value) {

    const number =
        Number(value);

    if (!isFinite(number)) {
        return "0.00";
    }

    return number.toFixed(2);
}

let purchaseDefaultExchangeRate = 1.0;

// The pending receipts currently rendered in the Pending Pricing
// form, kept here (rather than round-tripped through an inline
// onclick) so submitPurchaseInvoice() can look each selected row's
// procurementOrderID/materialID/receiptIndex/receivedQuantity back up
// by index without re-fetching or re-encoding them into HTML.
let purchasePendingCache = [];

async function loadPurchaseDefaultExchangeRate() {

    try {

        const response =
            await fetch("/api/purchase/config");

        const data =
            await response.json();

        purchaseDefaultExchangeRate =
            Number(data.exchangeRate) || 1.0;
    }
    catch (error) {

        console.error(error);

        purchaseDefaultExchangeRate = 1.0;
    }

    return purchaseDefaultExchangeRate;
}

// ============================================================
// PENDING PRICING
// ============================================================

async function showPurchasePending() {

    const content =
        document.getElementById(
            "purchase-content"
        );

    content.innerHTML = `
        <p>Loading Pending Pricing...</p>
    `;

    await loadPurchaseDefaultExchangeRate();

    try {

        const response =
            await fetch("/api/purchase/pending");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const pending =
            data.pending || [];

        if (pending.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    No deliveries waiting on a price. Received
                    Procurement deliveries appear here as soon as
                    Goods Receipt records them.
                </div>
            `;

            return;
        }

        purchasePendingCache = pending;

        otherCostRowCounters["purchpending"] = 0;

        content.innerHTML =
            renderPurchasePendingForm(pending);
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

function renderPurchasePendingForm(pending) {

    let rows = "";

    for (let i = 0; i < pending.length; i++) {

        const receipt = pending[i];

        rows += `
            <tr>

                <td>
                    <input
                        type="checkbox"
                        id="purchpending-${i}-select"
                        onchange="togglePurchasePendingRow(${i})"
                    >
                </td>

                <td>${escapeHtml(receipt.procurementOrderID)}</td>

                <td>
                    ${escapeHtml(receipt.materialID)}
                    <br>
                    <small>${escapeHtml(receipt.materialName || "")}</small>
                </td>

                <td>${escapeHtml(receipt.supplierName || "No Supplier")}</td>

                <td>${receipt.warehouseID}</td>

                <td>${escapeHtml(receipt.receiptDate)}</td>

                <td>${receipt.receivedQuantity}</td>

                <td>
                    ${receipt.hasPreviousPrice
                        ? formatMoney(receipt.previousUnitPriceEUR) + " EUR"
                        : "<small>No previous price</small>"}
                </td>

                <td>
                    <input
                        type="number"
                        min="0"
                        step="0.01"
                        id="purchpending-${i}-cost"
                        class="purchase-cost-input"
                        placeholder="Unit Cost"
                        disabled
                    >
                </td>

                <td>
                    <select
                        id="purchpending-${i}-quote"
                        class="purchase-quote-select"
                        style="max-width: 280px;"
                        onchange="applyPendingQuote(${i})"
                        disabled
                    >
                        <option value="">- none -</option>
                    </select>
                </td>

            </tr>
        `;
    }

    return `

        <div class="form-container form-container-wide">

            <h2>
                Price Received Deliveries
            </h2>

            <small>
                Select one or more deliveries below and enter each
                one's per-unit cost - the total is calculated for you.
                Several deliveries, even from different Procurement
                Orders, can share a single invoice (a consolidated
                shipment). Customs and freight are shared out across
                the selected deliveries proportionally to each one's
                own total material cost (unit cost × quantity).
            </small>

            <label>
                Invoice Date
            </label>

            <input type="date" id="purchpending-date">

            <label>
                Currency
            </label>

            <select id="purchpending-currency"
                    onchange="handlePurchaseCurrencyChange()">
                <option value="EUR">EUR</option>
                <option value="USD">USD</option>
            </select>

            <label>
                Exchange Rate (USD → EUR)
            </label>

            <input
                type="number"
                min="0.000001"
                step="0.0001"
                id="purchpending-rate"
                value="1.0"
                disabled
            >

            <label>
                Customs Cost (Zoll) - whole invoice
            </label>

            <input
                type="number"
                min="0"
                step="0.01"
                id="purchpending-customs"
                value="0"
            >

            <label>
                Freight / Transport Cost - whole invoice
            </label>

            <input
                type="number"
                min="0"
                step="0.01"
                id="purchpending-freight"
                value="0"
            >

            <div class="multi-line-items">

                <div class="multi-line-header">

                    <label>
                        Other Costs (optional)
                    </label>

                    <button
                        type="button"
                        class="add-line-button"
                        onclick="addOtherCostRow('purchpending', 'purchpending-othercosts', null, 'pending')">

                        + Add Other Cost

                    </button>

                </div>

                <small>
                    For a one-time charge that is neither customs nor
                    freight - e.g. a machinery setup fee paid once to a
                    contractor. It is amortized as a per-unit addition
                    to that Supplier's materials, not prorated across
                    this invoice's own lines.
                </small>

                <div id="purchpending-othercosts">
                </div>

            </div>

            <label>
                Comment (optional)
            </label>

            <input
                type="text"
                id="purchpending-comment"
                placeholder="e.g. Supplier invoice number"
            >

        </div>

        <h2>
            Deliveries
        </h2>

        <div class="material-table-container">

            <table class="material-table">

                <thead>
                    <tr>
                        <th></th>
                        <th>Order</th>
                        <th>Material</th>
                        <th>Supplier</th>
                        <th>Warehouse</th>
                        <th>Receipt Date</th>
                        <th>Received Qty</th>
                        <th>Previous Price (EUR)</th>
                        <th>Unit Cost</th>
                        <th>Quotation (optional)</th>
                    </tr>
                </thead>

                <tbody>
                    ${rows}
                </tbody>

            </table>

        </div>

        <div class="form-actions">

            <button onclick="submitPurchaseInvoice()">
                Create Invoice
            </button>

        </div>

        <div id="purchpending-message">
        </div>
    `;
}

// Red marking + warning when an amount is negative (a discount)
function flagNegativeAmount(input) {

    const negative = Number(input.value) < 0;

    input.classList.toggle("negative-input", negative);

    const warning = input.parentElement.querySelector(".negative-warning");

    if (warning) {
        warning.style.display = negative ? "" : "none";
    }
}

// Money text that turns red (with a minus-discount hint) when negative
function moneyWithSign(value, suffix) {

    const text = formatMoney(value) + (suffix || "");

    return Number(value) < 0 ?
        `<span class="negative-amount" title="Negative amount (discount)">&#9888; ${text}</span>` :
        text;
}

async function handlePurchaseCurrencyChange() {

    const currency =
        document.getElementById(
            "purchpending-currency"
        ).value;

    const rateInput =
        document.getElementById(
            "purchpending-rate"
        );

    if (currency === "EUR") {

        rateInput.value = 1.0;
        rateInput.disabled = true;
    }
    else {

        // always the rate saved in Settings (editable afterwards)
        rateInput.value = await loadPurchaseDefaultExchangeRate();
        rateInput.disabled = false;
    }
}

function togglePurchasePendingRow(index) {

    const checkbox =
        document.getElementById(
            `purchpending-${index}-select`
        );

    // One invoice = one Supplier (each Supplier bills separately):
    // checking a delivery deselects the ones of any other Supplier
    if (checkbox.checked) {

        const supplier =
            purchasePendingCache[index].supplierName || "";

        let deselected = false;

        for (let i = 0; i < purchasePendingCache.length; i++) {

            const other =
                document.getElementById(`purchpending-${i}-select`);

            if (i !== index && other && other.checked &&
                (purchasePendingCache[i].supplierName || "") !== supplier) {

                other.checked = false;

                togglePurchasePendingRow(i);

                deselected = true;
            }
        }

        const note = document.getElementById("purchpending-message");

        if (note && deselected) {

            note.textContent =
                "An invoice covers one Supplier only - deliveries of " +
                "other Suppliers were deselected.";
        }
    }

    const costInput =
        document.getElementById(
            `purchpending-${index}-cost`
        );

    costInput.disabled = !checkbox.checked;

    if (!checkbox.checked) {
        costInput.value = "";
    }

    // Quotation to price from (optional)
    const quoteSelect =
        document.getElementById(`purchpending-${index}-quote`);

    if (quoteSelect) {

        quoteSelect.disabled = !checkbox.checked;

        if (!checkbox.checked) {

            quoteSelect.value = "";
        }
        else if (!quoteSelect.dataset.loaded) {

            const receipt = purchasePendingCache[index];

            loadQuoteOptions(
                quoteSelect.id, receipt.materialID,
                receipt.receivedQuantity, "");
        }
    }

    // Any Other Cost row's "Quantity (this invoice)" hint depends on
    // which deliveries are checked - keep it in sync.
    if (typeof refreshAllOtherCostQuantities === "function") {
        refreshAllOtherCostQuantities("purchpending-othercosts");
    }
}

async function submitPurchaseInvoice() {

    const pending =
        purchasePendingCache;

    const message =
        document.getElementById(
            "purchpending-message"
        );

    const date =
        document.getElementById(
            "purchpending-date"
        ).value;

    const currency =
        document.getElementById(
            "purchpending-currency"
        ).value;

    // Always 1.0 for EUR, regardless of whatever the (disabled, and
    // possibly stale from an earlier USD selection) Exchange Rate
    // field still holds - a EUR invoice priced at anything but its
    // own currency's 1:1 rate would silently under/over-value every
    // price derived from it.
    const exchangeRate =
        currency === "EUR" ?
        1.0 :
        Number(
            document.getElementById(
                "purchpending-rate"
            ).value
        );

    const customsCost =
        Number(
            document.getElementById(
                "purchpending-customs"
            ).value
        ) || 0;

    const freightCost =
        Number(
            document.getElementById(
                "purchpending-freight"
            ).value
        ) || 0;

    const comment =
        document.getElementById(
            "purchpending-comment"
        ).value.trim();


    if (!date) {

        message.textContent =
            "Please enter an Invoice Date.";

        return;
    }

    if (!exchangeRate || exchangeRate <= 0) {

        message.textContent =
            "Exchange Rate must be greater than zero.";

        return;
    }


    const lines = [];

    for (let i = 0; i < pending.length; i++) {

        const checkbox =
            document.getElementById(
                `purchpending-${i}-select`
            );

        if (!checkbox || !checkbox.checked) {
            continue;
        }

        const costInput =
            document.getElementById(
                `purchpending-${i}-cost`
            );

        const unitCost =
            Number(costInput.value);

        if (!costInput.value || unitCost < 0) {

            message.textContent =
                `Enter a valid Unit Cost for ${pending[i].materialID} ` +
                `(Order ${pending[i].procurementOrderID}).`;

            return;
        }

        lines.push({
            procurementOrderID: pending[i].procurementOrderID,
            materialID: pending[i].materialID,
            receiptIndex: pending[i].receiptIndex,
            receivedQuantity: pending[i].receivedQuantity,
            unitCost: unitCost,
            quotationID:
                (document.getElementById(`purchpending-${i}-quote`) || {})
                    .value || ""
        });
    }

    if (lines.length === 0) {

        message.textContent =
            "Select at least one delivery to price.";

        return;
    }

    const otherCostsResult =
        collectOtherCostsFromForm(
            "purchpending", "purchpending-othercosts");

    if (otherCostsResult.error) {

        message.textContent =
            otherCostsResult.error;

        return;
    }


    const payload = {
        date: date,
        currency: currency,
        exchangeRate: exchangeRate,
        customsCost: customsCost,
        freightCost: freightCost,
        comment: comment,
        lines: lines,
        otherCosts: otherCostsResult.otherCosts
    };

    try {

        const response =
            await fetch(
                "/api/purchase/invoices/create",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify(payload)
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        await showPurchasePending();
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// "OTHER COSTS" - REPEATABLE ROWS (one-time Supplier-tied charges,
// e.g. a machinery setup fee paid once to a contractor, amortized as
// a per-unit addition to that Supplier's materials rather than
// prorated across this invoice's own lines "by value" the way
// customs/freight are - see PurchaseInvoiceOtherCost on the backend).
// Shared markup/logic reused by the "Price Received Deliveries" form,
// the "Adjust Material Price" form, and the Edit Invoice card.
// ============================================================

let otherCostRowCounters = {};

// Maps a rendered Other Cost row's prefix to where its quantity hint
// (below) should read its lines from - "pending" (the "Price Received
// Deliveries" form's checked rows), "invoice:<id>" (an existing
// invoice being edited), or omitted/falsy for a form with no sensible
// source (e.g. "Adjust Material Price"), which simply gets no hint.
let otherCostRowSources = {};

function otherCostRowHtml(formPrefix, rowIndex, data, source) {

    data = data || {};

    const rowPrefix =
        `${formPrefix}-othercost-${rowIndex}`;

    return `
        <div class="line-item cost-row-wrap" id="${rowPrefix}-row">

            <div class="line-item-field cost-supplier-field" style="display:none;">

                <label>
                    Supplier
                </label>

                ${supplierComboboxHtml(rowPrefix, "Search supplier by name...")}

            </div>

            <div class="cost-table-row cols-other">

                <div class="cost-cell">

                    <input
                        type="number"
                        step="0.01"
                        id="${rowPrefix}-amount"
                        placeholder="0.00"
                        value="${data.amountEUR != null ? data.amountEUR : ""}"
                        class="${data.amountEUR < 0 ? "negative-input" : ""}"
                        oninput="flagNegativeAmount(this)"
                    >

                    <div class="negative-warning"
                         style="${data.amountEUR < 0 ? "" : "display:none;"}">
                        &#9888; Negative amount: a discount / credit that
                        lowers the cost
                    </div>

                </div>

                <div class="cost-cell">

                    <select id="${rowPrefix}-scope">
                        <option value="invoice" ${!data.supplierWide ? "selected" : ""}>
                            Only this invoice
                        </option>
                        <option value="supplier" ${data.supplierWide ? "selected" : ""}>
                            All purchases to this Supplier
                        </option>
                    </select>

                    ${source ? `
                        <div id="${rowPrefix}-qty" class="empty-message">
                            Select a Supplier
                        </div>
                    ` : ""}

                </div>

                <div class="cost-cell">

                    <input
                        type="text"
                        id="${rowPrefix}-comment"
                        placeholder="e.g. Machinery setup fee"
                        value="${escapeHtml(data.comment || "")}"
                    >

                </div>

                <button
                    type="button"
                    class="remove-line-button"
                    title="Remove this Other Cost"
                    onclick="document.getElementById('${rowPrefix}-row').remove()">

                    &times;

                </button>

            </div>
        </div>
    `;
}

function addOtherCostRow(formPrefix, containerId, data, source) {

    if (otherCostRowCounters[formPrefix] === undefined) {
        otherCostRowCounters[formPrefix] = 0;
    }

    const rowIndex =
        otherCostRowCounters[formPrefix]++;

    const container =
        document.getElementById(containerId);

    if (!container) {
        return;
    }

    // Table look: one header row (hidden while there are no rows)
    if (!container.querySelector(".cost-table-head")) {

        container.classList.add("cost-table");

        const head = document.createElement("div");

        head.className =
            "cost-table-head cols-other";

        head.innerHTML =
            "<div>Amount (EUR)</div><div>Amortize across</div>" +
            "<div>Comment</div><div></div>";

        container.insertBefore(head, container.firstChild);
    }

    const wrapper =
        document.createElement("div");

    wrapper.innerHTML =
        otherCostRowHtml(formPrefix, rowIndex, data, source).trim();

    const rowElement =
        wrapper.firstElementChild;

    container.appendChild(rowElement);

    const rowPrefix =
        `${formPrefix}-othercost-${rowIndex}`;

    if (source) {
        otherCostRowSources[rowPrefix] = source;
    }

    // The Supplier is filled in asynchronously - the quantity hint
    // must be refreshed once it is there, not before
    initSupplierCombobox(
        rowPrefix,
        (data && data.supplierName) || "")
        .then(() => refreshOtherCostQuantity(rowPrefix));

    refreshOtherCostQuantity(rowPrefix);
}

// Recomputes and redisplays the "Quantity (this invoice)" hint for
// one Other Cost row, from whatever source addOtherCostRow recorded
// for it - the quantity of the row's chosen Supplier's materials
// among that form's own lines right now. This is the number the
// backend actually divides the Amount by for an invoice-only Other
// Cost, or feeds into the running cumulative total for a
// Supplier-wide one (see PurchaseManager::getLineOtherCostAdditionEUR).
function refreshOtherCostQuantity(rowPrefix) {

    const hint =
        document.getElementById(`${rowPrefix}-qty`);

    if (!hint) {
        // No hint element for this row (no source was given, e.g.
        // the Adjust Material Price form) - nothing to refresh.
        return;
    }

    const supplierInput =
        document.getElementById(`${rowPrefix}-supplier`);

    const source =
        otherCostRowSources[rowPrefix];

    // The Other Cost belongs to the Supplier of the deliveries on
    // this invoice - when they all come from one Supplier it is taken
    // from there and the Supplier field is not asked for at all (it
    // only shows when the invoice mixes several Suppliers)
    const invoiceSuppliers = [];

    const addInvoiceSupplier = name => {
        if (name && !invoiceSuppliers.includes(name)) {
            invoiceSuppliers.push(name);
        }
    };

    if (source === "pending") {

        for (let i = 0; i < purchasePendingCache.length; i++) {

            const checkbox =
                document.getElementById(`purchpending-${i}-select`);

            if (checkbox && checkbox.checked) {
                addInvoiceSupplier(purchasePendingCache[i].supplierName);
            }
        }
    }
    else if (source && source.startsWith("invoice:")) {

        const sourceInvoice =
            purchaseInvoiceCache[source.slice("invoice:".length)];

        if (sourceInvoice) {

            for (const line of sourceInvoice.lines) {
                addInvoiceSupplier(line.supplierName);
            }
        }
    }

    const supplierField =
        supplierInput ? supplierInput.closest(".line-item-field") : null;

    if (supplierInput && source) {

        if (invoiceSuppliers.length === 1) {

            const searchInput =
                document.getElementById(`${rowPrefix}-supplier-search`);

            supplierInput.value = invoiceSuppliers[0];

            if (searchInput) {
                searchInput.value = invoiceSuppliers[0];
            }

            if (supplierField) {
                supplierField.style.display = "none";
            }
        }
        else if (supplierField) {

            supplierField.style.display = "";
        }
    }

    const supplierName =
        supplierInput ? supplierInput.value.trim() : "";

    if (!supplierName) {
        hint.textContent =
            source && invoiceSuppliers.length === 0 ?
            "Select the deliveries to price first" :
            "Select a Supplier";
        return;
    }

    let quantity = 0;

    if (source === "pending") {

        for (let i = 0; i < purchasePendingCache.length; i++) {

            const checkbox =
                document.getElementById(`purchpending-${i}-select`);

            if (checkbox && checkbox.checked &&
                purchasePendingCache[i].supplierName === supplierName) {

                quantity += purchasePendingCache[i].receivedQuantity;
            }
        }
    }
    else if (source && source.startsWith("invoice:")) {

        const invoiceID =
            source.slice("invoice:".length);

        const invoice =
            purchaseInvoiceCache[invoiceID];

        if (invoice) {

            for (const line of invoice.lines) {

                if (line.supplierName === supplierName) {
                    quantity += line.receivedQuantity;
                }
            }
        }
    }

    hint.textContent =
        quantity > 0 ?
        `${quantity} unit(s) from ${supplierName} on this invoice` :
        `No ${supplierName} deliveries selected on this invoice yet`;
}

// Refreshes every Other Cost row's quantity hint under one container -
// called whenever something that could change the matched quantity
// changes (a delivery checkbox, a Supplier selection).
function refreshAllOtherCostQuantities(containerId) {

    const container =
        document.getElementById(containerId);

    if (!container) {
        return;
    }

    for (const row of container.querySelectorAll(".line-item")) {

        const rowPrefix =
            row.id.endsWith("-row") ?
            row.id.slice(0, -"-row".length) : row.id;

        refreshOtherCostQuantity(rowPrefix);
    }
}

// Reads every Other Cost row currently under containerId for the
// given formPrefix. Returns { otherCosts } on success, or { error }
// with a user-facing message on the first invalid row.
function collectOtherCostsFromForm(formPrefix, containerId) {

    const container =
        document.getElementById(containerId);

    if (!container) {
        return { otherCosts: [] };
    }

    const rows =
        container.querySelectorAll(".line-item");

    const otherCosts = [];

    for (const row of rows) {

        const rowPrefix =
            row.id.endsWith("-row") ?
            row.id.slice(0, -"-row".length) : row.id;

        const supplierInput =
            document.getElementById(`${rowPrefix}-supplier`);

        const supplierName =
            supplierInput ? supplierInput.value.trim() : "";

        // (empty is fine: an Other Cost belongs to the Supplier of
        // the invoice, which the server fills in)

        const amountInput =
            document.getElementById(`${rowPrefix}-amount`);

        const amountEUR =
            Number(amountInput.value);

        if (!amountInput.value || !isFinite(amountEUR)) {

            return {
                error:
                    `Enter a valid Amount (EUR) for the Other Cost.`
            };
        }

        const scope =
            document.getElementById(`${rowPrefix}-scope`).value;

        const comment =
            document.getElementById(`${rowPrefix}-comment`).value.trim();

        otherCosts.push({
            supplierName: supplierName,
            amountEUR: amountEUR,
            supplierWide: scope === "supplier",
            comment: comment
        });
    }

    return { otherCosts: otherCosts };
}

// ============================================================
// ADJUST MATERIAL PRICE (no Procurement delivery involved)
// ============================================================
// For a Material that needs its price set or corrected directly -
// e.g. a price known from another source, or a one-off fix - with no
// received delivery behind it. Internally this still creates a
// one-line Purchase Invoice with no customs/freight, so it shows up
// the same way in Invoices and in the Material's price history.

// Row counter for the dynamic Material rows below - always increases,
// even across removed rows, so two rows never share element IDs.
let purchAdjustRowCounter = 0;

async function showPurchaseAdjust() {

    const content =
        document.getElementById(
            "purchase-content"
        );

    content.innerHTML = `
        <p>Loading...</p>
    `;

    await loadPurchaseDefaultExchangeRate();

    purchAdjustRowCounter = 0;
    otherCostRowCounters["purchadjust"] = 0;

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                Adjust Material Price
            </h2>

            <small>
                Registers a price directly, with no Procurement delivery
                behind it - use this for a price known from another
                source, a one-off correction, or to set up initial
                prices for several Materials at once, the way a real
                supplier invoice with several materials, customs (Zoll)
                and freight/transport would arrive. For a delivery that
                actually arrived, price it from Pending Pricing instead,
                so it stays matched to its receipt.
            </small>

            <label>
                Date
            </label>

            <input type="date" id="purchadjust-date">

            <label>
                Currency
            </label>

            <select id="purchadjust-currency"
                    onchange="handlePurchaseAdjustCurrencyChange()">
                <option value="EUR">EUR</option>
                <option value="USD">USD</option>
            </select>

            <label>
                Exchange Rate (USD → EUR)
            </label>

            <input
                type="number"
                min="0.000001"
                step="0.0001"
                id="purchadjust-rate"
                value="1.0"
                disabled
            >

            <label>
                Customs Cost (Zoll) - whole invoice
            </label>

            <input
                type="number"
                min="0"
                step="0.01"
                id="purchadjust-customs"
                value="0"
            >

            <label>
                Freight / Transport Cost - whole invoice
            </label>

            <input
                type="number"
                min="0"
                step="0.01"
                id="purchadjust-freight"
                value="0"
            >

            <div class="multi-line-items">

                <div class="multi-line-header">

                    <label>
                        Other Costs (optional)
                    </label>

                    <button
                        type="button"
                        class="add-line-button"
                        onclick="addOtherCostRow('purchadjust', 'purchadjust-othercosts')">

                        + Add Other Cost

                    </button>

                </div>

                <small>
                    For a one-time charge that is neither customs nor
                    freight - e.g. a machinery setup fee paid once to a
                    contractor. It is amortized as a per-unit addition
                    to that Supplier's materials, not prorated across
                    this invoice's own lines.
                </small>

                <div id="purchadjust-othercosts">
                </div>

            </div>

            <label>
                Comment (optional)
            </label>

            <input
                type="text"
                id="purchadjust-comment"
                placeholder="e.g. Reason for the adjustment"
            >

        </div>

        <div class="multi-line-items">

            <div class="multi-line-header">

                <label>
                    Materials
                </label>

                <button
                    type="button"
                    class="add-line-button"
                    onclick="addPurchaseAdjustMaterialRow()">

                    + Add Material

                </button>

            </div>

            <small>
                Customs and freight above are shared out across the
                Materials below proportionally to each one's own total
                cost (unit cost × quantity), same as a normal invoice.
            </small>

            <div class="cost-table">

                <div class="cost-table-head cols-adjust">
                    <div>Material ID</div>
                    <div>Quantity</div>
                    <div>Unit Cost</div>
                    <div></div>
                </div>

                <div id="purchadjust-rows">
                </div>

            </div>

        </div>

        <div class="form-actions">
            <button onclick="submitPurchaseAdjust()">
                Save Prices
            </button>
        </div>

        <div id="purchadjust-message">
        </div>
    `;

    addPurchaseAdjustMaterialRow();
}

function addPurchaseAdjustMaterialRow() {

    const container =
        document.getElementById(
            "purchadjust-rows"
        );

    const rowIndex =
        purchAdjustRowCounter++;

    const prefix =
        `purchadjust-row-${rowIndex}`;

    const row =
        document.createElement("div");

    row.className =
        "line-item cost-table-row cols-adjust";

    row.id =
        `${prefix}-row`;

    row.dataset.prefix =
        prefix;

    row.innerHTML = `

        <div class="cost-cell">

            ${materialComboboxHtml(prefix, "Search by ID or name...")}

            <div id="${prefix}-material-id-currentprice" class="empty-message">
            </div>

        </div>

        <div class="cost-cell">

            <input
                type="number"
                min="0.001"
                step="any"
                id="${prefix}-quantity"
                placeholder="Qty"
                value="1"
            >

        </div>

        <div class="cost-cell">

            <input
                type="number"
                min="0"
                step="0.01"
                id="${prefix}-unitcost"
                placeholder="Unit Cost"
            >

        </div>

        <button
            type="button"
            class="remove-line-button"
            title="Remove this material"
            onclick="this.closest('.line-item').remove()">

            &times;

        </button>
    `;

    container.appendChild(row);

    initMaterialCombobox(prefix);
}

async function handlePurchaseAdjustCurrencyChange() {

    const currency =
        document.getElementById(
            "purchadjust-currency"
        ).value;

    const rateInput =
        document.getElementById(
            "purchadjust-rate"
        );

    if (currency === "EUR") {

        rateInput.value = 1.0;
        rateInput.disabled = true;
    }
    else {

        // always the rate saved in Settings (editable afterwards)
        rateInput.value = await loadPurchaseDefaultExchangeRate();
        rateInput.disabled = false;
    }
}

async function submitPurchaseAdjust() {

    const message =
        document.getElementById(
            "purchadjust-message"
        );

    const date =
        document.getElementById(
            "purchadjust-date"
        ).value;

    const currency =
        document.getElementById(
            "purchadjust-currency"
        ).value;

    // Always 1.0 for EUR - see the same guard in submitPurchaseInvoice()
    // for why the field itself cannot be trusted here.
    const exchangeRate =
        currency === "EUR" ?
        1.0 :
        Number(
            document.getElementById(
                "purchadjust-rate"
            ).value
        );

    const customsCost =
        Number(
            document.getElementById(
                "purchadjust-customs"
            ).value
        ) || 0;

    const freightCost =
        Number(
            document.getElementById(
                "purchadjust-freight"
            ).value
        ) || 0;

    const comment =
        document.getElementById(
            "purchadjust-comment"
        ).value.trim();


    if (!date) {

        message.textContent =
            "Please enter a Date.";

        return;
    }

    if (!exchangeRate || exchangeRate <= 0) {

        message.textContent =
            "Exchange Rate must be greater than zero.";

        return;
    }

    if (customsCost < 0 || freightCost < 0) {

        message.textContent =
            "Customs Cost and Freight Cost cannot be negative.";

        return;
    }


    const rows =
        document.querySelectorAll(
            "#purchadjust-rows .line-item"
        );

    if (rows.length === 0) {

        message.textContent =
            "Add at least one Material.";

        return;
    }


    const lines = [];

    const seenMaterialIDs = new Set();

    for (const row of rows) {

        const prefix =
            row.dataset.prefix;

        const materialID =
            document.getElementById(
                `${prefix}-material-id`
            ).value.trim();

        if (!materialID) {

            message.textContent =
                "Please select a Material for every row.";

            return;
        }

        if (seenMaterialIDs.has(materialID)) {

            message.textContent =
                `${materialID} was added more than once - combine ` +
                `it into a single row instead.`;

            return;
        }

        seenMaterialIDs.add(materialID);

        const quantityInput =
            document.getElementById(
                `${prefix}-quantity`
            );

        const quantity =
            Number(quantityInput.value);

        if (!quantityInput.value || quantity <= 0 ||
            !hasMaxThreeDecimals(quantity)) {

            message.textContent =
                `Enter a valid Quantity (max. 3 decimals) for ${materialID}.`;

            return;
        }

        const unitCostInput =
            document.getElementById(
                `${prefix}-unitcost`
            );

        const unitCost =
            Number(unitCostInput.value);

        if (!unitCostInput.value || unitCost < 0) {

            message.textContent =
                `Enter a valid Unit Cost for ${materialID}.`;

            return;
        }

        lines.push({
            procurementOrderID: "MANUAL",
            materialID: materialID,
            receiptIndex: -1,
            receivedQuantity: quantity,
            unitCost: unitCost
        });
    }

    const otherCostsResult =
        collectOtherCostsFromForm(
            "purchadjust", "purchadjust-othercosts");

    if (otherCostsResult.error) {

        message.textContent =
            otherCostsResult.error;

        return;
    }


    const payload = {
        date: date,
        currency: currency,
        exchangeRate: exchangeRate,
        customsCost: customsCost,
        freightCost: freightCost,
        comment: comment,
        lines: lines,
        otherCosts: otherCostsResult.otherCosts
    };

    try {

        const response =
            await fetch(
                "/api/purchase/invoices/create",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify(payload)
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        const data =
            await response.json();

        await showPurchaseAdjust();

        document.getElementById(
            "purchadjust-message"
        ).textContent =
            `Prices saved (Invoice ${data.id}).`;
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

// ============================================================
// INVOICES - LIST / DELETE
// ============================================================

// Invoices currently on screen, keyed by ID, so an invoice can be
// switched into edit mode and back without a round trip to the
// server just to re-render it.
let purchaseInvoiceCache = {};

async function showPurchaseInvoices() {

    const content =
        document.getElementById(
            "purchase-content"
        );

    content.innerHTML = `
        <p>Loading Invoices...</p>
    `;

    try {

        const response =
            await fetch("/api/purchase/invoices");

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const invoices =
            (data.invoices || []).slice().reverse();

        purchaseInvoiceCache = {};

        for (const invoice of invoices) {
            purchaseInvoiceCache[invoice.id] = invoice;
        }

        if (invoices.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    No Purchase Invoices yet. Price a delivery from
                    Pending Pricing to create the first one.
                </div>
            `;

            return;
        }

        content.innerHTML =
            invoices.map(
                invoice => renderPurchaseInvoiceCard(invoice, false)
            ).join("");

        // (each invoice is one collapsed line - click to open)
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// Renders one invoice card in either view mode (editing = false) or
// edit mode (editing = true) - the same card element, identified by
// id "purchinv-<invoiceID>", is swapped between the two in place, so
// editing one invoice never disturbs the rest of the list.

function renderPurchaseInvoiceCard(
    invoice, editing, previousPrices, expanded) {

    // Collapsed to one line in the list; editing (or coming back from
    // it) keeps the invoice open
    expanded = !!(editing || expanded);

    previousPrices = previousPrices || {};

    const escapedID =
        escapeHtml(invoice.id);

    let lineRows = "";

    let otherCostGrandTotalEUR = 0;

    for (let i = 0; i < invoice.lines.length; i++) {

        const line =
            invoice.lines[i];

        const isManual =
            line.procurementOrderID === "MANUAL";

        const previous =
            previousPrices[line.materialID];

        lineRows += `
            <tr>
                <td>
                    ${isManual
                        ? `<span class="status-badge status-inactive invoice-manual-badge">Manual adj.</span>`
                        : escapeHtml(line.procurementOrderID)}
                </td>
                <td>
                    <span style="white-space:nowrap;">${escapeHtml(line.materialID)}</span>
                    ${line.materialName ?
                        `<br><small style="color:#6b7280;">${escapeHtml(line.materialName)}</small>` : ""}
                </td>
                <td>${line.receivedQuantity}</td>
                ${editing
                    ? `<td>
                            ${previous
                                ? formatMoney(previous.unitPriceEUR) + " EUR"
                                : "<small>No previous price</small>"}
                       </td>`
                    : ""}
                <td>
                    ${editing
                        ? `<input
                                type="number"
                                min="0"
                                step="0.01"
                                id="purchinv-${escapedID}-line-${i}-cost"
                                class="purchase-cost-input"
                                value="${line.unitCost}"
                           >
                           ${isManual ? "" : `
                           <br>
                           <select
                                id="purchinv-${escapedID}-line-${i}-quote"
                                class="purchase-quote-select"
                                style="max-width: 240px; margin-top: 4px;"
                                onchange="applyInvoiceQuote('${escapedID}', ${i})"
                           >
                                <option value="${escapeHtml(line.quotationID || "")}">
                                    ${line.quotationID ?
                                        escapeHtml(line.quotationID) :
                                        "- no quotation -"}
                                </option>
                           </select>`}`
                        : formatMoney(line.unitCost) +
                            purchaseInvoiceQuoteNote(line, invoice.currency)}
                </td>
                <td>${formatMoney(line.materialCost)}</td>
                <td>${formatMoney(line.allocatedCost)}</td>
                <td>${formatMoney(line.unitPrice)}</td>
                <td>${formatMoney(line.unitPriceEUR)}</td>
                <td>${formatMoney(line.otherCostAdditionEUR || 0)}</td>
                <td>${moneyWithSign(line.otherCostTotalEUR || 0)}</td>
                <td>${formatMoney(line.landedUnitPriceEUR != null ?
                        line.landedUnitPriceEUR : line.unitPriceEUR)}</td>
            </tr>
        `;

        otherCostGrandTotalEUR += (line.otherCostTotalEUR || 0);
    }

    let otherCostsSummary = "";

    for (const otherCost of (invoice.otherCosts || [])) {

        otherCostsSummary += `
            <br>${otherCost.amountEUR < 0 ? "Discount / credit" : "Other Cost"} (${escapeHtml(otherCost.supplierName)}):
            ${moneyWithSign(otherCost.amountEUR, " EUR")} -
            ${otherCost.supplierWide ?
                "amortized across all purchases to this Supplier" :
                "amortized on this invoice only"}
            ${otherCost.comment ?
                " (" + escapeHtml(otherCost.comment) + ")" : ""}
        `;
    }

    // Supplier-wide Other Costs entered on EARLIER invoices also land
    // here - listed so the total above is never a mystery number
    for (const inherited of (invoice.inheritedOtherCosts || [])) {

        otherCostsSummary += `
            <br>Other Cost carried over from ${escapeHtml(inherited.sourceInvoiceID)}
            (${escapeHtml(inherited.supplierName)}):
            ${moneyWithSign(inherited.amountEUR, " EUR")} spread over
            ${inherited.quantityBasis} units bought from this Supplier
            up to this invoice -
            ${formatMoney(inherited.shareEUR)} EUR on this invoice
            ${inherited.comment ?
                " (" + escapeHtml(inherited.comment) + ")" : ""}
        `;
    }

    if (otherCostGrandTotalEUR !== 0) {

        otherCostsSummary += `
            <br>Total Other Costs applied to this invoice:
            ${moneyWithSign(otherCostGrandTotalEUR, " EUR")}
        `;
    }

    const header = editing
        ? `
            <div class="procurement-inline-form">

                <label>
                    Date
                    <input type="date" id="purchinv-${escapedID}-date"
                           value="${escapeHtml(invoice.date)}">
                </label>

                <label>
                    Currency
                    <select id="purchinv-${escapedID}-currency"
                            onchange="handlePurchaseInvoiceEditCurrencyChange('${escapedID}')">
                        <option value="EUR" ${invoice.currency === "EUR" ? "selected" : ""}>EUR</option>
                        <option value="USD" ${invoice.currency === "USD" ? "selected" : ""}>USD</option>
                    </select>
                </label>

                <label>
                    Exchange Rate
                    <input type="number" min="0.000001" step="0.0001"
                           id="purchinv-${escapedID}-rate"
                           value="${invoice.currency === "EUR" ? 1.0 : invoice.exchangeRate}"
                           ${invoice.currency === "EUR" ? "disabled" : ""}>
                </label>

                <label>
                    Customs Cost
                    <input type="number" min="0" step="0.01"
                           id="purchinv-${escapedID}-customs"
                           value="${invoice.customsCost}">
                </label>

                <label>
                    Freight Cost
                    <input type="number" min="0" step="0.01"
                           id="purchinv-${escapedID}-freight"
                           value="${invoice.freightCost}">
                </label>

                <label>
                    Comment
                    <input type="text" id="purchinv-${escapedID}-comment"
                           value="${escapeHtml(invoice.comment || "")}">
                </label>

            </div>

            <div class="multi-line-items">

                <div class="multi-line-header">

                    <label>
                        Other Costs (optional)
                    </label>

                    <button
                        type="button"
                        class="add-line-button"
                        onclick="addOtherCostRow('invedit-${escapedID}', 'purchinv-${escapedID}-othercosts', null, 'invoice:${escapedID}')">

                        + Add Other Cost

                    </button>

                </div>

                <small>
                    For a one-time charge that is neither customs nor
                    freight - amortized as a per-unit addition to that
                    Supplier's materials, not prorated across this
                    invoice's own lines.
                </small>

                <div id="purchinv-${escapedID}-othercosts">
                </div>

            </div>
        `
        : `
            <p>
                Exchange Rate: ${invoice.exchangeRate}
                &nbsp;|&nbsp;
                Customs: ${formatMoney(invoice.customsCost)}
                &nbsp;|&nbsp;
                Freight: ${formatMoney(invoice.freightCost)}
                &nbsp;|&nbsp;
                Total Material Cost: ${formatMoney(invoice.totalMaterialCost)}
                ${invoice.comment
                    ? `<br><strong>Comment: ${escapeHtml(invoice.comment)}</strong>`
                    : ""}
                ${otherCostsSummary}
            </p>
        `;

    const actions = editing
        ? `
            <div class="form-actions">
                <button onclick="savePurchaseInvoiceEdit('${escapedID}')">
                    Save and Close
                </button>
                <button class="button-muted"
                        onclick="cancelEditPurchaseInvoice('${escapedID}')">
                    Cancel
                </button>
            </div>

            <div id="purchinv-${escapedID}-message">
            </div>
        `
        : `
            <div class="form-actions">
                <button onclick="toggleEditPurchaseInvoice('${escapedID}')">
                    Edit
                </button>
                <button class="button-muted"
                        onclick="deletePurchaseInvoice('${escapedID}')">
                    Delete Invoice
                </button>
            </div>
        `;

    const invoiceSupplier =
        invoice.lines.length > 0 ? (invoice.lines[0].supplierName || "") : "";

    // Total of the invoice (material + customs + freight, in the
    // invoice currency; the EUR equivalent when it is USD)
    const invoiceTotal =
        invoice.totalMaterialCost + invoice.customsCost + invoice.freightCost;

    const summaryTotal =
        `<strong>Total ${formatMoney(invoiceTotal)} ${escapeHtml(invoice.currency)}` +
        (invoice.currency === "USD" ?
            ` (${formatMoney(invoiceTotal * invoice.exchangeRate)} EUR)` : "") +
        `</strong>`;

    const summaryExtra =
        `Material ${formatMoney(invoice.totalMaterialCost)}` +
        (invoice.customsCost > 0 ?
            ` &middot; Customs ${formatMoney(invoice.customsCost)}` : "") +
        (invoice.freightCost > 0 ?
            ` &middot; Freight ${formatMoney(invoice.freightCost)}` : "") +
        (otherCostGrandTotalEUR !== 0 ?
            ` &middot; ${otherCostGrandTotalEUR < 0 ? "Discount" : "Other"} ${moneyWithSign(otherCostGrandTotalEUR, " EUR")}` : "");

    return `

        <div class="procurement-card ${expanded ? "" : "collapsed"}"
             id="purchinv-${escapedID}">

            <div class="procurement-summary"
                 onclick="togglePurchaseInvoiceCard('${escapedID}')">

                <span class="procurement-summary-id">${escapedID}</span>

                <span class="procurement-summary-qty">
                    ${escapeHtml(invoice.date)}
                </span>

                <span class="status-badge status-ordered">
                    ${escapeHtml(invoice.currency)}
                </span>

                <span class="procurement-summary-material">
                    ${escapeHtml(invoiceSupplier)}
                    &nbsp;&middot;&nbsp;
                    ${invoice.lines.length} line${invoice.lines.length === 1 ? "" : "s"}
                </span>

                <span class="procurement-summary-qty">
                    ${summaryExtra}
                </span>

                <span class="procurement-summary-qty" style="color:#111827;">
                    ${summaryTotal}
                </span>

                ${editing
                    ? `<span class="status-badge status-partial">Editing</span>`
                    : ""}

                <span class="procurement-summary-caret">&#9656;</span>

            </div>

            <div class="purchase-invoice-body" id="purchinv-${escapedID}-body"
                 style="padding: 4px 16px 16px;" ${expanded ? "" : "hidden"}>

            ${header}

            <div class="material-table-container">

                <table class="material-table invoice-lines-table">

                    <thead>
                        <tr>
                            <th>Order</th>
                            <th>Material</th>
                            <th>Qty</th>
                            ${editing ? `<th title="Previous Price (EUR)">Prev. EUR</th>` : ""}
                            <th title="Unit Cost">Unit Cost</th>
                            <th title="Material Cost (unit cost x quantity)">Cost</th>
                            <th title="Allocated Customs/Freight">Cust./Frt.</th>
                            <th title="Unit Price">Price</th>
                            <th title="Unit Price (EUR)">Price EUR</th>
                            <th title="Other Cost per unit (EUR)">Other/unit</th>
                            <th title="Other Cost Total (EUR)">Other tot.</th>
                            <th title="Landed Price (EUR)">Landed EUR</th>
                        </tr>
                    </thead>

                    <tbody>
                        ${lineRows}
                    </tbody>

                </table>

            </div>

            ${actions}

            </div>

        </div>
    `;
}

function togglePurchaseInvoiceCard(id) {

    const card = document.getElementById(`purchinv-${id}`);

    const body = document.getElementById(`purchinv-${id}-body`);

    if (!card || !body) {
        return;
    }

    body.hidden = !body.hidden;

    card.classList.toggle("collapsed", body.hidden);
}

async function handlePurchaseInvoiceEditCurrencyChange(id) {

    const currency =
        document.getElementById(
            `purchinv-${id}-currency`
        ).value;

    const rateInput =
        document.getElementById(
            `purchinv-${id}-rate`
        );

    if (currency === "EUR") {

        rateInput.value = 1.0;
        rateInput.disabled = true;
    }
    else {

        // coming from EUR (rate fixed at 1): propose the rate saved
        // in Settings; a rate already typed for a USD invoice stays
        if (rateInput.disabled || Number(rateInput.value) === 1) {
            rateInput.value = await loadPurchaseDefaultExchangeRate();
        }

        rateInput.disabled = false;
    }
}

// Fetches, for each distinct material on this invoice, the last
// price on record from a DIFFERENT invoice - so editing invoice X
// shows what the material cost before X, not X's own (about to be
// re-entered) value. Returns { materialID: { unitPriceEUR, date } }.

async function loadPreviousPricesForInvoice(invoice) {

    const previousPrices = {};

    const materialIDs =
        [...new Set(invoice.lines.map(line => line.materialID))];

    for (const materialID of materialIDs) {

        try {

            const response =
                await fetch(
                    `/api/purchase/materials/${encodeURIComponent(materialID)}/history`
                );

            if (!response.ok) {
                continue;
            }

            const data =
                await response.json();

            const otherEntries =
                (data.history || []).filter(
                    entry => entry.sourceInvoiceID !== invoice.id
                );

            if (otherEntries.length > 0) {

                previousPrices[materialID] =
                    otherEntries[otherEntries.length - 1];
            }
        }
        catch (error) {
            console.error(error);
        }
    }

    return previousPrices;
}

async function toggleEditPurchaseInvoice(id) {

    const invoice =
        purchaseInvoiceCache[id];

    const card =
        document.getElementById(
            `purchinv-${id}`
        );

    if (!invoice || !card) {
        return;
    }

    const previousPrices =
        await loadPreviousPricesForInvoice(invoice);

    // The card may have been removed (e.g. the user navigated away)
    // while the price lookups above were in flight.
    const stillThere =
        document.getElementById(`purchinv-${id}`);

    if (!stillThere) {
        return;
    }

    const wrapper =
        document.createElement("div");

    wrapper.innerHTML =
        renderPurchaseInvoiceCard(invoice, true, previousPrices).trim();

    stillThere.replaceWith(wrapper.firstElementChild);

    invoice.lines.forEach((line, lineIndex) => {

        if (line.procurementOrderID !== "MANUAL") {

            loadQuoteOptions(
                `purchinv-${id}-line-${lineIndex}-quote`,
                line.materialID, line.receivedQuantity,
                line.quotationID || "");
        }
    });

    otherCostRowCounters[`invedit-${id}`] = 0;

    for (const otherCost of (invoice.otherCosts || [])) {

        addOtherCostRow(
            `invedit-${id}`,
            `purchinv-${id}-othercosts`,
            otherCost,
            `invoice:${id}`);
    }
}

function cancelEditPurchaseInvoice(id) {

    const invoice =
        purchaseInvoiceCache[id];

    const card =
        document.getElementById(
            `purchinv-${id}`
        );

    if (!invoice || !card) {
        return;
    }

    const wrapper =
        document.createElement("div");

    wrapper.innerHTML =
        renderPurchaseInvoiceCard(invoice, false, null, true).trim();

    card.replaceWith(wrapper.firstElementChild);
}

async function savePurchaseInvoiceEdit(id) {

    const invoice =
        purchaseInvoiceCache[id];

    const message =
        document.getElementById(
            `purchinv-${id}-message`
        );

    if (!invoice) {
        return;
    }

    const date =
        document.getElementById(
            `purchinv-${id}-date`
        ).value;

    const currency =
        document.getElementById(
            `purchinv-${id}-currency`
        ).value;

    // Always 1.0 for EUR - see the same guard in submitPurchaseInvoice()
    // for why the field itself cannot be trusted here. This is also
    // what lets re-saving an existing invoice correct a bad rate that
    // was stored before this guard existed.
    const exchangeRate =
        currency === "EUR" ?
        1.0 :
        Number(
            document.getElementById(
                `purchinv-${id}-rate`
            ).value
        );

    const customsCost =
        Number(
            document.getElementById(
                `purchinv-${id}-customs`
            ).value
        ) || 0;

    const freightCost =
        Number(
            document.getElementById(
                `purchinv-${id}-freight`
            ).value
        ) || 0;

    const comment =
        document.getElementById(
            `purchinv-${id}-comment`
        ).value.trim();

    if (!date) {

        message.textContent =
            "Please enter a Date.";

        return;
    }

    if (!exchangeRate || exchangeRate <= 0) {

        message.textContent =
            "Exchange Rate must be greater than zero.";

        return;
    }

    const lines = [];

    for (let i = 0; i < invoice.lines.length; i++) {

        const costInput =
            document.getElementById(
                `purchinv-${id}-line-${i}-cost`
            );

        const unitCost =
            Number(costInput.value);

        if (!costInput.value || unitCost < 0) {

            message.textContent =
                `Enter a valid Unit Cost for ${invoice.lines[i].materialID}.`;

            return;
        }

        lines.push({
            procurementOrderID: invoice.lines[i].procurementOrderID,
            materialID: invoice.lines[i].materialID,
            receiptIndex: invoice.lines[i].receiptIndex,
            receivedQuantity: invoice.lines[i].receivedQuantity,
            unitCost: unitCost,
            quotationID:
                (document.getElementById(`purchinv-${id}-line-${i}-quote`) ||
                    { value: invoice.lines[i].quotationID || "" }).value
        });
    }

    const otherCostsResult =
        collectOtherCostsFromForm(
            `invedit-${id}`, `purchinv-${id}-othercosts`);

    if (otherCostsResult.error) {

        message.textContent =
            otherCostsResult.error;

        return;
    }

    const payload = {
        id: id,
        date: date,
        currency: currency,
        exchangeRate: exchangeRate,
        customsCost: customsCost,
        freightCost: freightCost,
        comment: comment,
        lines: lines,
        otherCosts: otherCostsResult.otherCosts
    };

    try {

        const response =
            await fetch(
                "/api/purchase/invoices/update",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify(payload)
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        // Refresh from the server so the recalculated allocated
        // cost/unit price show immediately, then close (view mode).
        await showPurchaseInvoices();
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}

async function deletePurchaseInvoice(id) {

    const confirmed =
        await showConfirmDialog(
            `Delete Purchase Invoice ${id}? Its deliveries return to ` +
            `Pending Pricing and its price history entries disappear.`
        );

    if (!confirmed) {
        return;
    }

    try {

        const response =
            await fetch(
                "/api/purchase/invoices/delete",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({ id: id })
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            alert(errorMessage);

            return;
        }

        await showPurchaseInvoices();
    }
    catch (error) {

        console.error(error);

        alert("Could not connect to the server.");
    }
}

// ============================================================
// MATERIAL PRICE HISTORY
// ============================================================

async function showPurchaseHistory() {

    const content =
        document.getElementById(
            "purchase-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                Material Price History
            </h2>

            <label>
                Material
            </label>

            ${materialComboboxHtml("purchasehistory", "Search by ID or name...")}

            <div class="form-actions">
                <button onclick="loadPurchaseHistory()">
                    Show History
                </button>
            </div>

        </div>

        <div id="purchasehistory-result">
        </div>
    `;

    await initMaterialCombobox("purchasehistory");
}

async function loadPurchaseHistory() {

    const materialID =
        document.getElementById(
            "purchasehistory-material-id"
        ).value.trim();

    const result =
        document.getElementById(
            "purchasehistory-result"
        );

    if (!materialID) {

        result.innerHTML = `
            <p>Please select a Material.</p>
        `;

        return;
    }

    result.innerHTML = `
        <p>Loading price history...</p>
    `;

    try {

        const response =
            await fetch(
                `/api/purchase/materials/${encodeURIComponent(materialID)}/history`
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            result.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const history =
            (data.history || []).slice().reverse();

        if (history.length === 0) {

            result.innerHTML = `
                <div class="empty-message">
                    No price recorded yet for
                    ${escapeHtml(data.materialName || materialID)}.
                </div>
            `;

            return;
        }

        let rows = "";

        for (const entry of history) {

            const isManual =
                entry.sourceProcurementOrderID === "MANUAL";

            rows += `
                <tr>
                    <td>${escapeHtml(entry.date)}</td>
                    <td>${formatMoney(entry.unitPriceEUR)}</td>
                    <td>${escapeHtml(entry.originalCurrency)}</td>
                    <td>${formatMoney(entry.originalUnitPrice)}</td>
                    <td>${entry.exchangeRateUsed}</td>
                    <td>${escapeHtml(entry.sourceInvoiceID)}</td>
                    <td>
                        ${isManual
                            ? `<span class="status-badge status-inactive">Manual Adjustment</span>`
                            : escapeHtml(entry.sourceProcurementOrderID)}
                    </td>
                </tr>
            `;
        }

        result.innerHTML = `

            <h3>
                ${escapeHtml(data.materialName || materialID)}
            </h3>

            <p>
                Current Unit Price:
                <strong>${formatMoney(data.currentUnitPriceEUR)} EUR</strong>
            </p>

            <div class="material-table-container">

                <table class="material-table">

                    <thead>
                        <tr>
                            <th>Date</th>
                            <th>Unit Price (EUR)</th>
                            <th>Original Currency</th>
                            <th>Original Unit Price</th>
                            <th>Exchange Rate Used</th>
                            <th>Invoice</th>
                            <th>Procurement Order</th>
                        </tr>
                    </thead>

                    <tbody>
                        ${rows}
                    </tbody>

                </table>

            </div>
        `;
    }
    catch (error) {

        console.error(error);

        result.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// ============================================================
// PRODUCT COST
// ============================================================

// Product Cost with estimates from quotations. "Heaters" is how many
// Products will be produced: each material is priced for its BOM
// quantity times that number (so the right quotation price tier
// applies); the cost shown stays per ONE Product. "As of" rebuilds
// the cost on a past date from the records dated on or before it
// (the Bill of Materials is always the current one).

let productCostQuantity = 1;

let productCostDate = "";

// "usual" = quotation-governed materials are priced at the usual
// purchase quantity marked on the quotation; "quantity" = at the
// quantity needed (BOM x heaters)
let productCostBasis = "usual";

async function showPurchaseProductCost() {

    const content =
        document.getElementById(
            "purchase-content"
        );

    content.innerHTML = `

        <div style="max-width: 900px;">

            <h2>Product Cost</h2>

            <!-- Quantity / date / Calculate are hidden for now (cost of
                 1 heater, today); the controls stay in the page so they
                 can be shown again by removing display:none -->
            <div class="form-actions" style="display: none; gap: 12px; flex-wrap: wrap; align-items: end;">

                <div>
                    <label>Heaters to produce</label>
                    <input type="number" id="pcost-qty" min="1" step="any"
                        style="width: 140px;"
                        value="${productCostQuantity}">
                </div>

                <div>
                    <label>Cost as of (optional)</label>
                    <input type="date" id="pcost-date"
                        value="${escapeHtml(productCostDate)}">
                </div>

                <!-- (the "Price basis" selector is hidden for now: the
                     usual purchase quantity of the quotation is used) -->

                <button onclick="applyProductCostControls()">
                    Calculate
                </button>

                <button class="button-muted" onclick="resetProductCostControls()">
                    Today
                </button>

            </div>

            <div id="pcost-note"></div>

        </div>

        <div id="pcost-results"></div>
    `;

    await loadPurchaseProductCostResults();
}

function applyProductCostControls() {

    const qty = Number(document.getElementById("pcost-qty").value);

    productCostQuantity = qty > 0 ? qty : 1;

    productCostDate = document.getElementById("pcost-date").value || "";

    loadPurchaseProductCostResults();
}

function resetProductCostControls() {

    productCostQuantity = 1;

    productCostDate = "";

    productCostBasis = "usual";

    document.getElementById("pcost-qty").value = 1;

    document.getElementById("pcost-date").value = "";

    loadPurchaseProductCostResults();
}

async function loadPurchaseProductCostResults() {

    const content =
        document.getElementById(
            "pcost-results"
        );

    const note = document.getElementById("pcost-note");

    if (note) {

        note.innerHTML = productCostDate ? `
            <p><small>
                Cost as of ${escapeHtml(productCostDate)}: each material
                uses the latest invoice or quotation dated on or before
                that day. The Bill of Materials is the current one (its
                changes are not kept in history).
            </small></p>` : "";
    }

    content.innerHTML = `
        <p>Loading Product Cost...</p>
    `;

    try {

        const response =
            await fetch(
                `/api/purchase/products/cost?quantity=${encodeURIComponent(productCostQuantity)}` +
                (productCostDate ? `&date=${encodeURIComponent(productCostDate)}` : "") +
                `&basis=${productCostBasis}`);

        if (!response.ok) {

            const errorMessage =
                await response.text();

            content.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        const products =
            data.products || [];

        if (products.length === 0) {

            content.innerHTML = `
                <div class="empty-message">
                    No Products defined yet.
                </div>
            `;

            return;
        }

        productCostList = products;

        renderProductCostTiles();
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// Products as photo tiles (same look as the main menu); a click opens
// the material list of that Product (Product Cost card), "Back" returns.
let productCostList = [];

function renderProductCostTiles() {

    const content = document.getElementById("pcost-results");

    if (!content) {
        return;
    }

    content.innerHTML = `
        <div class="product-tiles">
            ${productCostList.map((product, index) => `
                <div class="product-tile" onclick="openProductCostDetail(${index})">
                    <div class="product-tile-photo">
                        ${product.photo ?
                            `<img src="${escapeHtml(product.photo)}" alt="">` :
                            `<span class="product-tile-noimg">No photo</span>`}
                    </div>
                    <div class="product-tile-body">
                        <h3>${escapeHtml(product.productName)}</h3>
                        <p>${escapeHtml(product.productID)}</p>
                        <p>
                            <strong>${formatMoney(product.totalCostEUR)} EUR</strong>
                            <span class="status-badge ${product.complete ? "status-active" : "status-inactive"}">
                                ${product.complete ? "Complete" : "Incomplete"}
                            </span>
                        </p>
                        <label class="product-tile-upload" onclick="event.stopPropagation()">
                            ${product.photo ? "Change photo" : "Add photo"}
                            <input type="file" accept="image/*" style="display:none;"
                                onchange="uploadProductPhoto(${index}, this)">
                        </label>
                    </div>
                </div>
            `).join("")}
        </div>
    `;
}

// Search box of the Product Cost detail: hides the material rows that
// do not match (ID, name, category, supplier / part numbers...)
async function filterProductCostRows(input) {

    const section = input.closest(".procurement-section");

    if (!section) {
        return;
    }

    const text = input.value.trim().toLowerCase();

    const key = normalizeMaterialKey(text);

    const materials = await loadMaterialCache();

    const byID = {};

    for (const material of (materials || [])) {
        byID[material.id] = material;
    }

    let shown = 0;

    const rows = section.querySelectorAll("tbody tr[data-mid]");

    for (const row of rows) {

        const material = byID[row.dataset.mid];

        const haystack = row.textContent.toLowerCase() + " " +
            (material ? materialSearchText(material) : "");

        const match = text === "" ||
            haystack.includes(text) ||
            (key !== "" && normalizeMaterialKey(haystack).includes(key));

        row.dataset.searchHidden = match ? "" : "1";

        applyRowVisibility(row);

        if (match) {
            shown++;
        }
    }

    const count = section.querySelector(".product-cost-search-count");

    if (count) {
        count.textContent = text === "" ? "" :
            `${shown} of ${rows.length} materials`;
    }
}

function openProductCostDetail(index) {

    const content = document.getElementById("pcost-results");

    const product = productCostList[index];

    if (!content || !product) {
        return;
    }

    content.innerHTML = `
        <button class="button-muted" style="margin-bottom: 12px;"
            onclick="renderProductCostTiles()">&larr; All Products</button>
        ${product.photo ?
            `<img class="product-detail-photo" src="${escapeHtml(product.photo)}" alt="">` : ""}
        ${renderPurchaseProductCostCard(product)}
    `;
}

// Saves a photo for the Product with this ID, then calls onDone().
async function uploadProductPhotoById(productID, input, onDone) {

    const file = input.files[0];

    if (!file) {
        return;
    }

    try {

        const photoData = await readFileAsBase64(file);

        const response = await fetch(
            `/api/products/${encodeURIComponent(productID)}/photo`,
            {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ photoData: photoData })
            });

        if (!response.ok) {

            alert("Could not save photo: " + await response.text());

            return;
        }

        if (onDone) {
            onDone();
        }
    }
    catch (error) {

        console.error(error);

        alert("Could not save photo.");
    }
}

async function uploadProductPhoto(index, input) {

    const file = input.files[0];

    const product = productCostList[index];

    if (!file || !product) {
        return;
    }

    try {

        const photoData = await readFileAsBase64(file);

        const response = await fetch(
            `/api/products/${encodeURIComponent(product.productID)}/photo`,
            {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ photoData: photoData })
            });

        if (!response.ok) {

            alert("Could not save photo: " + await response.text());

            return;
        }

        const data = await response.json();

        product.photo = data.photo;

        renderProductCostTiles();
    }
    catch (error) {

        console.error(error);

        alert("Could not save photo.");
    }
}

// Totals of a Product, put at the top of its Excel export
function productCostSummaryRows(product) {

    const rows = [
        ["Product", `${product.productID} - ${product.productName}`],
        ["Total Cost (EUR)", Number(product.totalCostEUR).toFixed(2)]
    ];

    if (product.hasEstimates) {

        rows.push(["Real cost - invoices (EUR)",
            Number(product.realCostEUR).toFixed(2)]);

        rows.push(["Estimated cost - quotations (EUR)",
            Number(product.estimatedCostEUR).toFixed(2)]);
    }

    rows.push(["Status", product.complete ? "Complete" :
        "Incomplete - at least one BOM material has no price history yet"]);

    return rows;
}

// detailed = false: the price report (price and where it comes from, short).
// detailed = true : Review Product, with every comment on each price.
function renderPurchaseProductCostCard(product, detailed) {

    let lineRows = "";

    for (const line of product.lines) {

        // Internal Supplier material (made by 4Tex): grey, and not
        // "missing" a price - its cost is inside other materials
        const internalOnly = line.internal && !line.hasPrice;

        lineRows += `
            <tr class="${line.internal ? "internal-row" : ""}"
                data-mid="${escapeHtml(line.materialID)}">
                <td>
                    <a href="#"
                        onclick="openMaterialFromQuotation('${escapeHtml(line.materialID)}'); return false;">
                        ${escapeHtml(line.materialID)}</a>
                    ${line.materialName ?
                        "&mdash; " + escapeHtml(line.materialName) : ""}
                </td>
                <td data-export-value="${line.quantity}" data-export-unit="${escapeHtml(line.uom || "")}">
                    ${line.quantity}${line.uom ? " <strong>" + escapeHtml(line.uom) + "</strong>" : ""}
                    ${line.totalQuantity !== line.quantity ?
                        `<br><small>priced for ${line.totalQuantity}${line.uom ? " " + escapeHtml(line.uom) : ""}</small>` : ""}
                </td>
                <td data-export-value="${internalOnly || !line.hasPrice ? "" : Number(line.unitPriceEUR).toFixed(4).replace(/0+$/, "").replace(/\.$/, "")}">
                    ${internalOnly ?
                        "<small>Internal - cost is in other materials</small>" :
                        renderCostSourceHtml(line.estimate, line.totalQuantity, line.uom, !detailed)}
                </td>
                <td>
                    ${internalOnly ? "-" : formatMoney(line.lineCostEUR)}
                    ${line.estimate && line.estimate.estimated ?
                        `<br><small style="color:#b45309;">estimated</small>` : ""}
                </td>
            </tr>
        `;
    }

    return `

        <div class="procurement-section">

            <h3>
                ${escapeHtml(product.productID)}
                &nbsp;&mdash;&nbsp;
                ${escapeHtml(product.productName)}
                &nbsp;
                <span class="status-badge ${product.complete ? "status-active" : "status-inactive"}">
                    ${product.complete ? "Complete" : "Incomplete"}
                </span>
            </h3>

            <p data-summary="${escapeHtml(JSON.stringify(productCostSummaryRows(product)))}">
                Total Cost:
                <strong>${formatMoney(product.totalCostEUR)} EUR</strong>
                ${product.hasEstimates ?
                    `<br><small>
                        Real (invoices): ${formatMoney(product.realCostEUR)} EUR
                        &nbsp;|&nbsp;
                        <span style="color:#b45309;">Estimated (quotations):
                        ${formatMoney(product.estimatedCostEUR)} EUR</span>
                    </small>` : ""}
                ${!product.complete
                    ? `<br><small>Partial - at least one BOM material
                        has no price history yet.</small>`
                    : ""}
            </p>

            <input type="text" class="product-cost-search"
                placeholder="Search material (ID, name, part number...)"
                oninput="filterProductCostRows(this)">

            <small class="product-cost-search-count"></small>

            <div class="material-table-container material-table-container-fit">

                <table class="material-table material-table-sticky">

                    <thead>
                        <tr>
                            <th>Material</th>
                            <th>Quantity</th>
                            <th>Unit Price (EUR)</th>
                            <th>Line Cost (EUR)</th>
                        </tr>
                    </thead>

                    <tbody>
                        ${lineRows}
                    </tbody>

                </table>

            </div>

        </div>
    `;
}

// ============================================================
// PURCHASE - COST BY CATEGORY (pie chart)
// ============================================================
// Looks up one Product (same searchable combobox as elsewhere) and
// breaks its landed BOM cost down by each Material's Category,
// reusing the same /api/purchase/products/<id>/cost endpoint the
// Product Cost tab already uses - just grouped client-side instead of
// listed line by line. A Material with no Category set is grouped
// under "Other" (see the backend route). Drawn as a plain inline SVG
// pie chart - no charting library dependency.

async function showPurchaseCostByCategory() {

    const content =
        document.getElementById(
            "purchase-content"
        );

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                Cost by Category
            </h2>

            <label>
                Product
            </label>

            ${productComboboxHtml("costcategory", "Search by ID or name...")}

            <div class="form-actions">
                <button onclick="loadPurchaseCostByCategory()">
                    Show Breakdown
                </button>
            </div>

        </div>

        <div id="costcategory-result">
        </div>
    `;

    await initProductCombobox("costcategory");
}

// ============================================================
// PURCHASE - REVIEW PRODUCT (every comment on the prices)
// ============================================================
// Same material list as Product Cost, but each price keeps all its
// comments (quotation / invoice, import costs, usual quantity,
// warnings...), so the Product Cost report itself stays clean.

async function showPurchaseReviewProduct() {

    const content = document.getElementById("purchase-content");

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>
                Review Product
            </h2>

            <p>
                Every price of the Product with all its comments.
            </p>

            <label>
                Product
            </label>

            ${productComboboxHtml("reviewproduct", "Search by ID or name...")}

            <div class="form-actions">
                <button onclick="loadPurchaseReviewProduct()">
                    Review
                </button>
            </div>

        </div>

        <div id="reviewproduct-result">
        </div>
    `;

    await initProductCombobox("reviewproduct");
}

async function loadPurchaseReviewProduct() {

    const productID =
        document.getElementById("reviewproduct-product-id").value.trim();

    const result = document.getElementById("reviewproduct-result");

    if (!productID) {

        result.innerHTML = `<p>Please select a Product.</p>`;

        return;
    }

    result.innerHTML = `<p>Loading...</p>`;

    try {

        const response = await fetch(
            `/api/purchase/products/${encodeURIComponent(productID)}/cost` +
            `?quantity=1&basis=usual`);

        if (!response.ok) {

            result.innerHTML =
                `<p>Error: ${escapeHtml(await response.text())}</p>`;

            return;
        }

        result.innerHTML =
            renderPurchaseProductCostCard(await response.json(), true);
    }
    catch (error) {

        console.error(error);

        result.innerHTML = `<p>Could not connect to the server.</p>`;
    }
}

async function loadPurchaseCostByCategory() {

    const productID =
        document.getElementById(
            "costcategory-product-id"
        ).value.trim();

    const result =
        document.getElementById(
            "costcategory-result"
        );

    if (!productID) {

        result.innerHTML = `
            <p>Please select a Product.</p>
        `;

        return;
    }

    result.innerHTML = `
        <p>Loading cost breakdown...</p>
    `;

    try {

        const response =
            await fetch(
                `/api/purchase/products/${encodeURIComponent(productID)}/cost`
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            result.innerHTML = `
                <p>Error: ${escapeHtml(errorMessage)}</p>
            `;

            return;
        }

        const data =
            await response.json();

        result.innerHTML =
            renderPurchaseCostByCategory(data);
    }
    catch (error) {

        console.error(error);

        result.innerHTML = `
            <p>Could not connect to the server.</p>
        `;
    }
}

// A fixed palette, reused in order per Category (cycling if there are
// more Categories than colors) - no per-Category color is stored
// anywhere, so the same Category can land on a different color across
// two lookups if the set of Categories involved changes.

const PURCHASE_CATEGORY_COLORS = [
    "#2563eb", "#16a34a", "#d97706", "#dc2626", "#7c3aed",
    "#0891b2", "#db2777", "#65a30d", "#ea580c", "#4f46e5"
];

// Lines of the Product shown in the breakdown and the Category of
// each legend row, so a click can list the materials of that Category
let purchaseCategoryLines = [];

let purchaseCategoryNames = [];

function togglePurchaseCategoryDetail(index) {

    const row = document.getElementById(`purchcat-row-${index}`);

    if (!row) {
        return;
    }

    const existing = document.getElementById(`purchcat-detail-${index}`);

    if (existing) {

        existing.remove();

        row.classList.remove("category-open");

        return;
    }

    const category = purchaseCategoryNames[index];

    const lines = purchaseCategoryLines.filter(
        line => (line.category || "Other") === category);

    const categoryTotal = lines.reduce(
        (sum, line) => sum + (line.hasPrice ? line.lineCostEUR : 0), 0);

    const body = lines
        .slice()
        .sort((a, b) => (b.lineCostEUR || 0) - (a.lineCostEUR || 0))
        .map(line => `
            <tr class="${line.internal ? "internal-row" : ""}">
                <td>
                    <a href="#"
                        onclick="openMaterialFromQuotation('${escapeHtml(line.materialID)}'); return false;">
                        ${escapeHtml(line.materialID)}</a>
                    ${line.materialName ? "<br><small>" + escapeHtml(line.materialName) + "</small>" : ""}
                </td>
                <td>${line.quantity}${line.uom ? " " + escapeHtml(line.uom) : ""}</td>
                <td>${line.hasPrice ? formatMoney(line.unitPriceEUR) +
                        (line.uom ? ` <small>EUR / ${escapeHtml(line.uom)}</small>` : "") : "<small>No price yet</small>"}
                    ${line.hasPrice && line.estimate && line.estimate.estimated ?
                        `<br><small style="color:#b45309;">estimated</small>` : ""}</td>
                <td>${line.hasPrice ? formatMoney(line.lineCostEUR) : "-"}</td>
                <td>${line.hasPrice && categoryTotal > 0 ?
                    (line.lineCostEUR / categoryTotal * 100).toFixed(1) + "%" : "-"}</td>
            </tr>
        `).join("");

    const detail = document.createElement("tr");

    detail.id = `purchcat-detail-${index}`;

    detail.className = "category-detail-row";

    detail.innerHTML = `
        <td colspan="3">
            <table class="material-table category-detail-table">
                <thead>
                    <tr>
                        <th>Material</th>
                        <th>Qty</th>
                        <th>Unit price (EUR)</th>
                        <th>Cost (EUR)</th>
                        <th>% of category</th>
                    </tr>
                </thead>
                <tbody>${body}</tbody>
            </table>
        </td>
    `;

    row.after(detail);

    row.classList.add("category-open");
}

function renderPurchaseCostByCategory(product) {

    purchaseCategoryLines = product.lines || [];

    purchaseCategoryNames = [];

    const categoryTotals = {};

    // Only priced lines contribute - same as Product Cost's Total
    // Cost, an incomplete Product's breakdown reflects only what is
    // actually known.

    for (const line of (product.lines || [])) {

        if (!line.hasPrice) {
            continue;
        }

        const category =
            line.category || "Other";

        categoryTotals[category] =
            (categoryTotals[category] || 0) + line.lineCostEUR;
    }

    const categories =
        Object.keys(categoryTotals)
            .sort((a, b) => categoryTotals[b] - categoryTotals[a]);

    const headerHtml = `
        <h3>
            ${escapeHtml(product.productID)}
            &nbsp;&mdash;&nbsp;
            ${escapeHtml(product.productName)}
            &nbsp;
            <span class="status-badge ${product.complete ? "status-active" : "status-inactive"}">
                ${product.complete ? "Complete" : "Incomplete"}
            </span>
        </h3>

        ${product.hasEstimates
            ? `<p><small>Includes prices estimated from quotations
                (${formatMoney(product.estimatedCostEUR)} EUR of the total).</small></p>`
            : ""}

        ${!product.complete
            ? `<p><small>Partial - at least one BOM material has no
                price history yet, so it is left out of this
                breakdown.</small></p>`
            : ""}
    `;

    if (categories.length === 0) {

        return `
            ${headerHtml}

            <div class="empty-message">
                No priced BOM materials yet - nothing to break down by
                Category.
            </div>
        `;
    }

    const pricedTotal =
        categories.reduce(
            (sum, category) => sum + categoryTotals[category], 0);

    // Pie slices as SVG arc paths, one per Category, clockwise from
    // the top (12 o'clock, i.e. -90 degrees in SVG's angle system).

    const radius = 90;
    const centerX = 100;
    const centerY = 100;

    let cumulativeAngle = -90;
    let slicesHtml = "";
    let legendHtml = "";

    categories.forEach((category, index) => {

        const amount =
            categoryTotals[category];

        const percentage =
            pricedTotal > 0 ? (amount / pricedTotal) * 100 : 0;

        const color =
            PURCHASE_CATEGORY_COLORS[
                index % PURCHASE_CATEGORY_COLORS.length];

        if (categories.length === 1) {

            // A single Category is 100% of the pie - a 360 degree arc
            // is degenerate in SVG, so this draws a full circle
            // instead of an arc path.

            slicesHtml = `
                <circle cx="${centerX}" cy="${centerY}" r="${radius}"
                    fill="${color}" style="cursor:pointer;"
                    onclick="togglePurchaseCategoryDetail(${index})">
                </circle>
            `;
        }
        else {

            const sweepAngle =
                pricedTotal > 0 ? (amount / pricedTotal) * 360 : 0;

            const startAngle = cumulativeAngle;
            const endAngle = cumulativeAngle + sweepAngle;

            const startPoint =
                purchaseCategoryPolarPoint(
                    centerX, centerY, radius, startAngle);

            const endPoint =
                purchaseCategoryPolarPoint(
                    centerX, centerY, radius, endAngle);

            const largeArcFlag =
                sweepAngle > 180 ? 1 : 0;

            slicesHtml += `
                <path
                    d="M ${centerX} ${centerY}
                       L ${startPoint.x} ${startPoint.y}
                       A ${radius} ${radius} 0 ${largeArcFlag} 1 ${endPoint.x} ${endPoint.y}
                       Z"
                    fill="${color}" style="cursor:pointer;"
                    onclick="togglePurchaseCategoryDetail(${index})">
                </path>
            `;

            cumulativeAngle = endAngle;
        }

        purchaseCategoryNames[index] = category;

        legendHtml += `
            <tr id="purchcat-row-${index}" class="category-row"
                title="Click to see the materials of this category"
                onclick="togglePurchaseCategoryDetail(${index})">
                <td>
                    <span class="category-swatch" style="background: ${color};"></span>
                    ${escapeHtml(category)}
                    <small style="color:#6b7280;">&#9662;</small>
                </td>
                <td>${percentage.toFixed(1)}%</td>
                <td>${formatMoney(amount)}</td>
            </tr>
        `;
    });

    return `

        ${headerHtml}

        <div class="purchase-category-layout">

            <svg width="200" height="200" viewBox="0 0 200 200"
                class="purchase-category-chart">

                ${slicesHtml}

            </svg>

            <table class="material-table purchase-category-legend">

                <thead>
                    <tr>
                        <th>Category</th>
                        <th>%</th>
                        <th>Cost (EUR)</th>
                    </tr>
                </thead>

                <tbody>
                    ${legendHtml}
                </tbody>

                <tfoot>
                    <tr>
                        <td><strong>Total</strong></td>
                        <td><strong>100%</strong></td>
                        <td><strong>${formatMoney(pricedTotal)}</strong></td>
                    </tr>
                </tfoot>

            </table>

        </div>
    `;
}

function purchaseCategoryPolarPoint(centerX, centerY, radius, angleInDegrees) {

    const angleInRadians =
        angleInDegrees * Math.PI / 180;

    return {
        x: centerX + radius * Math.cos(angleInRadians),
        y: centerY + radius * Math.sin(angleInRadians)
    };
}

// ============================================================
// SETTINGS - DEFAULT EXCHANGE RATE
// ============================================================

async function showPurchaseSettings() {

    const content =
        document.getElementById(
            "purchase-content"
        );

    content.innerHTML = `
        <p>Loading Settings...</p>
    `;

    const rate =
        await loadPurchaseDefaultExchangeRate();

    content.innerHTML = `

        <div class="form-container">

            <h2>
                Purchase Settings
            </h2>

            <label>
                Default USD → EUR Exchange Rate
            </label>

            <small>
                Used to pre-fill new USD invoices. Each invoice keeps
                its own rate once created, so changing this never
                affects past invoices or their price history.
            </small>

            <input
                type="number"
                min="0.000001"
                step="0.0001"
                id="purchsettings-rate"
                value="${rate}"
            >

            <div class="form-actions">
                <button onclick="submitPurchaseExchangeRate()">
                    Save
                </button>
            </div>

            <div id="purchsettings-message">
            </div>

        </div>
    `;
}

async function submitPurchaseExchangeRate() {

    const message =
        document.getElementById(
            "purchsettings-message"
        );

    const rate =
        Number(
            document.getElementById(
                "purchsettings-rate"
            ).value
        );

    if (!rate || rate <= 0) {

        message.textContent =
            "Exchange Rate must be greater than zero.";

        return;
    }

    try {

        const response =
            await fetch(
                "/api/purchase/config",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({ exchangeRate: rate })
                }
            );

        if (!response.ok) {

            const errorMessage =
                await response.text();

            message.textContent =
                errorMessage;

            return;
        }

        message.textContent =
            "Exchange rate updated.";
    }
    catch (error) {

        console.error(error);

        message.textContent =
            "Could not connect to the server.";
    }
}


// ============================================================
// LOGIN, ROLES AND USER MANAGEMENT
// ============================================================
// Three roles (enforced by the server - see AuthManager.h):
//   viewer - read only
//   editor - day-to-day work (create / modify / delete / receive ...)
//   admin  - everything, plus user management and system actions
//
// This block only handles the login screen, the user bar in the
// header and hiding the buttons a role may not use. Hiding is just
// convenience: the server rejects forbidden requests (403) anyway.

let currentUser = null;

// Every request goes through this wrapper, so an expired or missing
// session (401) always brings up the login screen, whichever part of
// the app made the request.

const nativeFetch = window.fetch.bind(window);

// Busy indicator: while any request is running longer than a moment, a
// "Loading..." layer covers the page, so the same button is not
// clicked again and again.
let pendingRequests = 0;

let busyTimer = null;

let busyHideTimer = null;

function busyStart() {

    pendingRequests++;

    clearTimeout(busyHideTimer);

    if (pendingRequests === 1) {

        busyTimer = setTimeout(() => {

            let layer = document.getElementById("busy-overlay");

            if (!layer) {

                layer = document.createElement("div");

                layer.id = "busy-overlay";

                layer.innerHTML = `
                    <div class="busy-box">
                        <div class="busy-spinner"></div>
                        <span>Loading, please wait...</span>
                    </div>`;

                document.body.appendChild(layer);
            }

            layer.style.display = "flex";

        }, 350);
    }
}

function busyEnd() {

    pendingRequests = Math.max(0, pendingRequests - 1);

    if (pendingRequests === 0) {
        clearTimeout(busyTimer);

        // Short grace period: sequential requests (one per material)
        // must not flash the overlay off between them - that gap was
        // enough to click the same button again.
        busyHideTimer = setTimeout(() => {

            if (pendingRequests !== 0) {
                return;
            }

            const layer = document.getElementById("busy-overlay");

            if (layer) {
                layer.style.display = "none";
            }
        }, 250);
    }
}

window.fetch = async function (...args) {

    busyStart();

    let response;

    try {

        response = await nativeFetch(...args);
    }
    finally {

        busyEnd();
    }

    if (response.status === 401) {

        const target =
            typeof args[0] === "string" ?
            args[0] : ((args[0] && args[0].url) || "");

        if (!target.includes("/api/auth/login")) {

            showLoginScreen(
                "Your session has ended. Please log in again.",
                false);
        }
    }

    return response;
};


function roleLabel(role) {

    if (role === "admin") return "Admin";
    if (role === "editor") return "Editor";

    return "Viewer";
}


// ---------------- Login screen ----------------

function showLoginScreen(message, isInfo) {

    currentUser = null;

    document.body.dataset.role = "";

    let overlay =
        document.getElementById("login-overlay");

    if (!overlay) {

        overlay =
            document.createElement("div");

        overlay.id = "login-overlay";

        overlay.className = "login-overlay";

        overlay.innerHTML = `
            <div class="login-box">

                <h2>Warehouse Management System</h2>

                <p class="login-subtitle">Please log in</p>

                <label for="login-username">Username</label>

                <input
                    type="text"
                    id="login-username"
                    autocomplete="username">

                <label for="login-password">Password</label>

                <input
                    type="password"
                    id="login-password"
                    autocomplete="current-password">

                <button type="button" id="login-submit">
                    Log in
                </button>

                <div id="login-message" class="login-message">
                </div>

            </div>
        `;

        document.body.appendChild(overlay);

        document.getElementById("login-submit")
            .addEventListener("click", submitLogin);

        overlay.addEventListener("keydown", (event) => {

            if (event.key === "Enter") {
                submitLogin();
            }
        });
    }

    const messageBox =
        document.getElementById("login-message");

    messageBox.textContent = message || "";

    messageBox.classList.toggle("login-message-info", !!isInfo);

    document.getElementById("login-password").value = "";

    overlay.classList.remove("hidden");

    document.getElementById("login-username").focus();

    updateUserBar();
}


async function submitLogin() {

    const username =
        document.getElementById("login-username").value.trim();

    const password =
        document.getElementById("login-password").value;

    const messageBox =
        document.getElementById("login-message");

    messageBox.classList.remove("login-message-info");

    if (!username || !password) {

        messageBox.textContent =
            "Please enter username and password.";

        return;
    }

    try {

        const response =
            await nativeFetch(
                "/api/auth/login",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json"
                    },
                    body: JSON.stringify({ username, password })
                }
            );

        if (!response.ok) {

            messageBox.textContent =
                await response.text();

            document.getElementById("login-password").value = "";

            return;
        }

        currentUser =
            await response.json();

        document.getElementById("login-username").value = "";

        document.getElementById("login-password").value = "";

        document.getElementById("login-overlay")
            .classList.add("hidden");

        onLoggedIn();
    }
    catch (error) {

        console.error(error);

        messageBox.textContent =
            "Could not connect to the server.";
    }
}


function onLoggedIn() {

    document.body.dataset.role = currentUser.role;

    // Back to the dashboard: whatever was open before a re-login may
    // belong to another role.

    closeModule();

    updateUserBar();

    startRoleObserver();

    applyRoleRestrictions();
}


async function logoutUser() {

    try {

        await nativeFetch(
            "/api/auth/logout",
            { method: "POST" }
        );
    }
    catch (error) {

        console.error(error);
    }

    showLoginScreen("You have been logged out.", true);
}


async function initAuth() {

    try {

        const response =
            await nativeFetch("/api/auth/me");

        if (response.ok) {

            currentUser =
                await response.json();

            onLoggedIn();

            return;
        }
    }
    catch (error) {

        console.error(error);
    }

    showLoginScreen("", false);
}


// ---------------- User bar (header) ----------------

function updateUserBar() {

    const statusBox =
        document.querySelector(".header .status");

    if (!statusBox) {
        return;
    }

    let bar =
        document.getElementById("user-bar");

    if (!bar) {

        bar =
            document.createElement("span");

        bar.id = "user-bar";

        bar.className = "user-bar";

        statusBox.appendChild(bar);
    }

    if (!currentUser) {

        bar.innerHTML = "";

        return;
    }

    bar.innerHTML = `
        <span class="user-bar-name">
            ${escapeHtml(currentUser.username)}
            <small>${roleLabel(currentUser.role)}</small>
        </span>

        ${currentUser.role === "admin" ? `
            <button type="button" onclick="showUserManagement()">
                Users
            </button>` : ""}

        <button type="button" onclick="showChangePasswordDialog()">
            Password
        </button>

        <button type="button" onclick="logoutUser()">
            Log out
        </button>
    `;
}


// ---------------- Role restrictions (cosmetic) ----------------

// Buttons that change data: hidden for Viewer.

const EDIT_ACTION_PATTERN = new RegExp(
    "^(" + [
        "create\\w*", "modify\\w*", "delete\\w*", "add\\w*",
        "remove\\w*", "save\\w*", "submit\\w*", "confirm\\w*",
        "register\\w*", "quick\\w*", "cancel\\w*", "closeOrderGroup",
        "toggleEdit\\w*", "sell\\w*", "transfer\\w*", "goods\\w*",
        "resetNumbering", "loadData",
        "loadMaterialFor\\w*", "loadProductForModify",
        "loadSupplierForModify",
        "showCreate\\w*", "showModify\\w*", "showDelete\\w*",
        "showGoods\\w*", "showTransferMaterial",
        "showNewProcurementOrder", "showNewProjection",
        "showSellProduct", "showPurchaseAdjust",
        "showProductSettings", "showPurchaseSettings",
        "showNewInventoryCount", "apply\\w*",
        "showNewQuotation", "showEditQuotation"
    ].join("|") + ")\\b"
);

// System-level actions: Admin only (also hidden for Editor).

const ADMIN_ACTION_PATTERN =
    /^(loadData|resetNumbering|saveDocumentsFolder)\b/;

// Inputs whose change saves something (settings switches, uploads).

const EDIT_CHANGE_PATTERN =
    /^(toggleAutoSaveAndLoad|setDataLogging|uploadProcurementDocument|uploadQuotationDocument)\b/;

const ADMIN_CHANGE_PATTERN =
    /^(toggleAutoSaveAndLoad|setDataLogging)\b/;


function applyRoleRestrictions() {

    if (!currentUser) {
        return;
    }

    const role = currentUser.role;

    // Undo anything hidden for a previously logged-in user (logging
    // out and in again does not reload the page), so e.g. Exit and
    // Load Data come back when an Admin logs in after an Editor.

    for (const element of document.querySelectorAll("[data-role-hidden]")) {

        element.style.display = "";

        element.removeAttribute("data-role-hidden");
    }

    if (role === "admin") {
        return;
    }

    for (const element of document.querySelectorAll("[onclick]")) {

        const handler =
            (element.getAttribute("onclick") || "").trim();

        // A handler can hold several statements ("x = null; showModify...()"):
        // each one is checked, not just the first
        const statements = handler.split(";").map(part => part.trim());

        const hide =
            statements.some(part => ADMIN_ACTION_PATTERN.test(part)) ||
            (role === "viewer" &&
                statements.some(part => EDIT_ACTION_PATTERN.test(part)));

        if (hide) {
            element.style.display = "none";
            element.setAttribute("data-role-hidden", "1");
        }
    }

    for (const element of document.querySelectorAll("[onchange]")) {

        const handler =
            (element.getAttribute("onchange") || "").trim();

        const hide =
            ADMIN_CHANGE_PATTERN.test(handler) ||
            (role === "viewer" && EDIT_CHANGE_PATTERN.test(handler));

        if (hide) {

            const target =
                element.closest("label") || element;

            target.style.display = "none";
            target.setAttribute("data-role-hidden", "1");
        }
    }
}


let roleObserver = null;

let roleObserverScheduled = false;

function startRoleObserver() {

    if (roleObserver) {
        return;
    }

    // The pages build their buttons dynamically, so restrictions are
    // re-applied (debounced) whenever the page content changes.

    roleObserver = new MutationObserver(() => {

        if (roleObserverScheduled) {
            return;
        }

        roleObserverScheduled = true;

        requestAnimationFrame(() => {

            roleObserverScheduled = false;

            applyRoleRestrictions();
        });
    });

    roleObserver.observe(
        document.body,
        { childList: true, subtree: true }
    );
}


// ---------------- Small dialog (password forms) ----------------

function showAuthDialog(title, fields, submitText, onSubmit) {

    const overlay =
        document.createElement("div");

    overlay.className = "app-modal-overlay";

    overlay.style.zIndex = "1500";

    overlay.innerHTML = `
        <div class="app-modal-box auth-dialog">

            <h3>${escapeHtml(title)}</h3>

            ${fields.map((field, index) => `
                <label for="auth-dialog-${index}">
                    ${escapeHtml(field.label)}
                </label>
                <input
                    type="password"
                    id="auth-dialog-${index}"
                    autocomplete="new-password">
            `).join("")}

            <div class="auth-dialog-message"></div>

            <div class="app-modal-actions">
                <button type="button" class="app-modal-button-secondary">
                    Cancel
                </button>
                <button type="button" class="app-modal-button-primary">
                    ${escapeHtml(submitText)}
                </button>
            </div>

        </div>
    `;

    document.body.appendChild(overlay);

    const buttons =
        overlay.querySelectorAll(".app-modal-actions button");

    const messageBox =
        overlay.querySelector(".auth-dialog-message");

    const close = () => overlay.remove();

    buttons[0].onclick = close;

    const submit = async () => {

        const values =
            fields.map((field, index) =>
                document.getElementById(`auth-dialog-${index}`).value);

        const problem = await onSubmit(values);

        if (problem) {

            messageBox.textContent = problem;

            return;
        }

        close();
    };

    buttons[1].onclick = submit;

    overlay.addEventListener("keydown", (event) => {

        if (event.key === "Enter") {
            submit();
        }

        if (event.key === "Escape") {
            close();
        }
    });

    document.getElementById("auth-dialog-0").focus();
}


async function postJson(url, payload) {

    const response =
        await fetch(
            url,
            {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            }
        );

    return {
        ok: response.ok,
        text: await response.text()
    };
}


function showChangePasswordDialog() {

    showAuthDialog(
        "Change password",
        [
            { label: "Current password" },
            { label: "New password (min. 8 characters)" },
            { label: "Repeat new password" }
        ],
        "Change password",
        async ([current, next, repeat]) => {

            if (next !== repeat) {
                return "The new passwords do not match.";
            }

            const result =
                await postJson(
                    "/api/auth/change-password",
                    { currentPassword: current, newPassword: next });

            if (!result.ok) {
                return result.text;
            }

            showLoginScreen(
                "Password changed. Please log in again.", true);

            return "";
        }
    );
}


// ---------------- User management (Admin) ----------------

async function showUserManagement() {

    const dashboard =
        document.querySelector(".dashboard");

    dashboard.style.display = "none";

    document.getElementById("module-view")
        .classList.remove("hidden");

    document.getElementById("module-content").innerHTML = `
        <h1>User Management</h1>

        <p>
            Admin: full access and user management.
            Editor: can create, modify, delete and receive.
            Viewer: read only.
        </p>

        <div id="user-management-body">
            Loading users...
        </div>
    `;

    await refreshUserManagement();
}


async function refreshUserManagement() {

    const body =
        document.getElementById("user-management-body");

    if (!body) {
        return;
    }

    const response =
        await fetch("/api/users");

    if (!response.ok) {

        body.textContent =
            "Error: " + await response.text();

        return;
    }

    const data =
        await response.json();

    const roleSelect = (selected, extra) => `
        <select ${extra}>
            ${["viewer", "editor", "admin"].map(role => `
                <option value="${role}"
                    ${role === selected ? "selected" : ""}>
                    ${roleLabel(role)}
                </option>`).join("")}
        </select>
    `;

    body.innerHTML = `
        <div class="material-table-container">

            <table class="material-table">
                <thead>
                    <tr>
                        <th>Username</th>
                        <th>Role</th>
                        <th>Actions</th>
                    </tr>
                </thead>
                <tbody>
                    ${data.users.map(user => `
                        <tr>
                            <td>${escapeHtml(user.username)}</td>
                            <td>
                                ${roleSelect(
                                    user.role,
                                    `data-user="${escapeHtml(user.username)}"
                                     onchange="changeUserRole(this.dataset.user, this.value)"`)}
                            </td>
                            <td>
                                <button type="button"
                                    class="user-row-button"
                                    data-user="${escapeHtml(user.username)}"
                                    onclick="resetUserPasswordDialog(this.dataset.user)">
                                    Reset password
                                </button>
                                <button type="button"
                                    class="user-row-button user-row-button-danger"
                                    data-user="${escapeHtml(user.username)}"
                                    onclick="removeUser(this.dataset.user)">
                                    Delete
                                </button>
                            </td>
                        </tr>
                    `).join("")}
                </tbody>
            </table>

        </div>

        <div class="form-container">

            <h2>Add user</h2>

            <label>Username</label>
            <input type="text" id="new-user-name"
                placeholder="letters, digits, . _ -">

            <label>Password</label>
            <input type="password" id="new-user-password"
                autocomplete="new-password"
                placeholder="at least 8 characters">

            <label>Role</label>
            ${roleSelect("viewer", `id="new-user-role"`)}

            <div class="form-actions">
                <button type="button" onclick="addUserFromForm()">
                    Add user
                </button>
            </div>

            <div id="user-management-message"></div>

        </div>
    `;
}


function setUserManagementMessage(text) {

    const box =
        document.getElementById("user-management-message");

    if (box) {
        box.textContent = text;
    }
}


async function addUserFromForm() {

    const result =
        await postJson(
            "/api/users/create",
            {
                username:
                    document.getElementById("new-user-name").value.trim(),
                password:
                    document.getElementById("new-user-password").value,
                role:
                    document.getElementById("new-user-role").value
            });

    if (!result.ok) {

        setUserManagementMessage(result.text);

        return;
    }

    await refreshUserManagement();

    setUserManagementMessage("User added.");
}


async function changeUserRole(username, role) {

    const result =
        await postJson(
            "/api/users/role",
            { username, role });

    await refreshUserManagement();

    setUserManagementMessage(
        result.ok ? "Role updated." : result.text);

    // Changing your own role takes effect on your next request.

    if (result.ok && currentUser &&
        username.toLowerCase() === currentUser.username) {

        initAuth();
    }
}


function resetUserPasswordDialog(username) {

    showAuthDialog(
        `Reset password for ${username}`,
        [{ label: "New password (min. 8 characters)" }],
        "Reset password",
        async ([password]) => {

            const result =
                await postJson(
                    "/api/users/reset-password",
                    { username, newPassword: password });

            if (!result.ok) {
                return result.text;
            }

            setUserManagementMessage(
                `Password for ${username} was reset. ` +
                "Their open sessions were closed.");

            return "";
        }
    );
}


async function removeUser(username) {

    const confirmed =
        await showConfirmDialog(
            `Delete user "${username}"? This cannot be undone.`);

    if (!confirmed) {
        return;
    }

    const result =
        await postJson(
            "/api/users/delete",
            { username });

    await refreshUserManagement();

    setUserManagementMessage(
        result.ok ? "User deleted." : result.text);
}


initAuth();


/* ============================================================
   INVENTORY COUNT (physical inventory of one Warehouse)
   An Open count is a draft: counted quantities are saved but the
   stock is untouched and the Warehouse is locked (no goods
   receipts / issues / transfers / sales). Apply sets the stock to
   the counted quantities and documents every change in the
   movement history as INVENTORY ADJUSTMENT; Cancel changes nothing.
============================================================ */

let inventoryCountState = { id: "", status: "", lines: [], reasons: [] };

function inventoryCountToday() {

    const now = new Date();

    const month = String(now.getMonth() + 1).padStart(2, "0");

    const day = String(now.getDate()).padStart(2, "0");

    return `${now.getFullYear()}-${month}-${day}`;
}

function inventoryCountIsViewer() {

    return !!currentUser && currentUser.role === "viewer";
}

function inventoryCountStatusClass(status) {

    if (status === "Open") return "status-partial";

    if (status === "Applied") return "status-active";

    return "status-inactive";
}

function inventoryCountFormat(value) {

    return String(roundQty3(value));
}

function setInventoryCountMessage(text, isError) {

    const message =
        document.getElementById("inventory-count-message");

    if (!message) {
        return;
    }

    message.textContent = text || "";

    message.style.color = isError ? "#b91c1c" : "#15803d";

    message.style.marginTop = "12px";

    message.style.fontWeight = "bold";
}

// ------------------------------------------------------------
// List
// ------------------------------------------------------------

let inventoryCountList = [];

async function showInventoryCounts(selectedStatus) {

    const content =
        document.getElementById("inventory-content");

    content.innerHTML = `<p>Loading inventory counts...</p>`;

    try {

        const response =
            await fetch("/api/inventory-counts");

        if (!response.ok) {

            content.innerHTML = `
                <p>Error: ${escapeHtml(await response.text())}</p>`;

            return;
        }

        const data = await response.json();

        inventoryCountList = data.counts || [];

        renderInventoryCountList(selectedStatus || "All");
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `<p>Could not connect to the server.</p>`;
    }
}

function renderInventoryCountList(statusFilter) {

    const content =
        document.getElementById("inventory-content");

    const counts =
        inventoryCountList.filter(
            count => statusFilter === "All" ||
                count.status === statusFilter);

    const statusOptions =
        ["All", "Open", "Applied", "Cancelled"].map(status => `
            <option value="${status}"
                ${status === statusFilter ? "selected" : ""}>
                ${status}
            </option>`).join("");

    let rows = "";

    for (const count of counts) {

        rows += `
            <tr>
                <td>
                    <a href="#"
                        onclick="openInventoryCount('${escapeHtml(count.id)}'); return false;">
                        ${escapeHtml(count.id)}
                    </a>
                </td>
                <td>${count.warehouseID} - ${escapeHtml(count.warehouseName)}</td>
                <td>
                    <span class="status-badge ${inventoryCountStatusClass(count.status)}">
                        ${escapeHtml(count.status)}
                    </span>
                </td>
                <td>${escapeHtml(count.startDate)}</td>
                <td>${escapeHtml(count.endDate || "-")}</td>
                <td>${count.lineCount}</td>
                <td>${count.countedCount}</td>
                <td>${count.status === "Applied" ? count.adjustedCount : "-"}</td>
                <td>${escapeHtml(count.comment || "")}</td>
            </tr>`;
    }

    if (counts.length === 0) {

        rows = `
            <tr>
                <td colspan="9">No inventory counts found.</td>
            </tr>`;
    }

    content.innerHTML = `

        <div class="form-container form-container-wide"
            style="max-width: 1100px;">

            <h2>Inventory Counts</h2>

            <p>
                A count stays Open (draft) until it is applied or
                cancelled. While it is Open, stock movements in that
                Warehouse are suspended.
            </p>

            <div class="form-actions" style="gap: 10px; flex-wrap: wrap;">

                <button onclick="showNewInventoryCount()">
                    New Inventory Count
                </button>

                <select id="inventory-count-filter"
                    onchange="renderInventoryCountList(this.value)"
                    style="width: auto;">
                    ${statusOptions}
                </select>

                <button class="button-muted"
                    onclick="exportInventoryCountListToExcel()">
                    Export to Excel
                </button>

            </div>

            <div class="material-table-container" style="margin-top: 16px;">
                <table class="material-table">
                    <thead>
                        <tr>
                            <th>Count</th>
                            <th>Warehouse</th>
                            <th>Status</th>
                            <th>Started</th>
                            <th>Finished</th>
                            <th>Materials</th>
                            <th>Counted</th>
                            <th>Adjusted</th>
                            <th>Comment</th>
                        </tr>
                    </thead>
                    <tbody>${rows}</tbody>
                </table>
            </div>

        </div>
    `;
}

function exportInventoryCountListToExcel() {

    const statusFilter =
        (document.getElementById("inventory-count-filter") || {}).value ||
        "All";

    const rows = [[
        "Count", "Warehouse ID", "Warehouse", "Status", "Started",
        "Finished", "Materials", "Counted", "Adjusted", "Comment"
    ]];

    for (const count of inventoryCountList) {

        if (statusFilter !== "All" && count.status !== statusFilter) {
            continue;
        }

        rows.push([
            count.id, count.warehouseID, count.warehouseName,
            count.status, count.startDate, count.endDate || "",
            count.lineCount, count.countedCount,
            count.status === "Applied" ? count.adjustedCount : "",
            count.comment || ""
        ]);
    }

    downloadCSV("inventory_counts.csv", rows);
}

// ------------------------------------------------------------
// New count
// ------------------------------------------------------------

function showNewInventoryCount() {

    const content =
        document.getElementById("inventory-content");

    content.innerHTML = `

        <div class="form-container form-container-wide">

            <h2>New Inventory Count</h2>

            <label>Warehouse</label>

            <select id="inventory-count-warehouse"></select>

            <label>Start date</label>

            <input type="date" id="inventory-count-start"
                value="${inventoryCountToday()}">

            <label>Comment (optional)</label>

            <textarea id="inventory-count-comment" rows="3"
                placeholder="e.g. Annual inventory, Shelf A-C"></textarea>

            <p>
                Opening the count suspends goods receipts, issues,
                transfers and sales in this Warehouse until the count
                is applied or cancelled.
            </p>

            <div class="form-actions">

                <button onclick="createInventoryCount()">
                    Open Count
                </button>

                <button class="button-muted"
                    onclick="showInventoryCounts()">
                    Back
                </button>

            </div>

            <div id="inventory-count-message"></div>

        </div>
    `;

    populateWarehouseSelect(
        "inventory-count-warehouse", "",
        { placeholder: "Select Warehouse", showID: true });
}

async function createInventoryCount() {

    const warehouseID =
        document.getElementById("inventory-count-warehouse").value;

    const startDate =
        document.getElementById("inventory-count-start").value;

    const comment =
        document.getElementById("inventory-count-comment").value.trim();

    if (!warehouseID) {

        setInventoryCountMessage("Please select a Warehouse.", true);

        return;
    }

    if (!startDate) {

        setInventoryCountMessage("Please enter the start date.", true);

        return;
    }

    const result =
        await postJson(
            "/api/inventory-counts/create",
            {
                warehouseID: parseInt(warehouseID, 10),
                startDate,
                comment
            });

    if (!result.ok) {

        setInventoryCountMessage(result.text, true);

        return;
    }

    const data = JSON.parse(result.text);

    await openInventoryCount(data.id, data.message);
}

// ------------------------------------------------------------
// Count sheet
// ------------------------------------------------------------

async function openInventoryCount(id, infoMessage, restore) {

    const content =
        document.getElementById("inventory-content");

    // Called from list links too - make sure the Inventory screen is up
    if (!content) {
        return;
    }

    try {

        const response =
            await fetch(`/api/inventory-counts/${encodeURIComponent(id)}`);

        if (!response.ok) {

            content.innerHTML = `
                <p>Error: ${escapeHtml(await response.text())}</p>`;

            return;
        }

        const count = await response.json();

        inventoryCountState = {
            id: count.id,
            status: count.status,
            lines: count.lines || [],
            reasons: count.reasons || [],
            warehouseID: count.warehouseID,
            warehouseName: count.warehouseName,
            startDate: count.startDate,
            endDate: count.endDate,
            comment: count.comment || ""
        };

        renderInventoryCountSheet();

        // Coming back from a material: put back what was typed but
        // not saved yet.
        if (restore) {
            restoreInventoryCountUnsaved();
        }

        if (infoMessage) {
            setInventoryCountMessage(infoMessage, false);
        }
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `<p>Could not connect to the server.</p>`;
    }
}

function inventoryCountReasonOptions(selected, enabled) {

    let html = `<option value="">-</option>`;

    for (const reason of inventoryCountState.reasons) {

        html += `
            <option value="${escapeHtml(reason)}"
                ${reason === selected ? "selected" : ""}>
                ${escapeHtml(reason)}
            </option>`;
    }

    return html;
}

function inventoryCountDifferenceHtml(line) {

    if (!line.counted) {
        return "-";
    }

    const diff = roundQty3(line.difference);

    if (diff === 0) {
        return "0";
    }

    return diff < 0 ?
        `<span style="color:#b91c1c;font-weight:bold;">${diff}</span>` :
        `<span style="color:#15803d;font-weight:bold;">+${diff}</span>`;
}

// Materials of an Internal Supplier (self-made, normally not stocked)
// are shown in another colour so they stand out in the list.

// "12,5" is accepted as 12.5
function inventoryCountParse(text) {

    return String(text).trim().replace(",", ".");
}

// Material ID as a link to the Material detail; Back returns here.
function inventoryCountMaterialLink(materialID) {

    return `<a href="#" class="material-id-link"
        title="View this Material"
        onclick="event.preventDefault(); openMaterialFromInventoryCount('${escapeHtml(materialID)}');">
        ${escapeHtml(materialID)}</a>`;
}

let inventoryCountUnsaved = null;

function openMaterialFromInventoryCount(materialID) {

    const state = inventoryCountState;

    // Remember what is typed (even if not saved) before leaving
    inventoryCountUnsaved = null;

    if (state.status === "Open" && !inventoryCountIsViewer()) {

        const snapshot = { id: state.id, comment: "", lines: {} };

        const general = document.getElementById("inventory-count-comment");

        snapshot.comment = general ? general.value : "";

        state.lines.forEach((line, index) => {

            const counted = document.getElementById(`cnt-counted-${index}`);

            const reason = document.getElementById(`cnt-reason-${index}`);

            const comment = document.getElementById(`cnt-comment-${index}`);

            if (counted) {

                snapshot.lines[line.materialID] = {
                    counted: counted.value,
                    reason: reason ? reason.value : "",
                    comment: comment ? comment.value : ""
                };
            }
        });

        inventoryCountUnsaved = snapshot;
    }

    openModule("materials");

    materialsReturnInventoryCount = state.id;

    showMaterialDetail(materialID);
}

function restoreInventoryCountUnsaved() {

    const snapshot = inventoryCountUnsaved;

    inventoryCountUnsaved = null;

    if (!snapshot || snapshot.id !== inventoryCountState.id ||
        inventoryCountState.status !== "Open") {
        return;
    }

    const general = document.getElementById("inventory-count-comment");

    if (general) {
        general.value = snapshot.comment;
    }

    inventoryCountState.lines.forEach((line, index) => {

        const saved = snapshot.lines[line.materialID];

        const counted = document.getElementById(`cnt-counted-${index}`);

        if (!saved || !counted) {
            return;
        }

        counted.value = saved.counted;

        updateInventoryCountRow(index);

        const reason = document.getElementById(`cnt-reason-${index}`);

        if (reason && !reason.disabled) {
            reason.value = saved.reason;
        }

        const comment = document.getElementById(`cnt-comment-${index}`);

        if (comment) {
            comment.value = saved.comment;
        }
    });
}

function inventoryCountPhotoHtml(line) {

    return line.photo ?
        `<img src="/${escapeHtml(line.photo)}"
            class="material-thumbnail" alt="Material photo">` :
        "-";
}

function inventoryCountRowStyle(line) {

    return line.internalSupplier ?
        `style="background: #ede9fe;" title="Internal Supplier material"` :
        "";
}

function renderInventoryCountSheet() {

    const content =
        document.getElementById("inventory-content");

    const state = inventoryCountState;

    const editable =
        state.status === "Open" && !inventoryCountIsViewer();

    let rows = "";

    state.lines.forEach((line, index) => {

        const system = roundQty3(line.systemQuantity);

        const countedText =
            line.counted ? inventoryCountFormat(line.countedQuantity) : "";

        if (editable) {

            rows += `
                <tr id="cnt-row-${index}" ${inventoryCountRowStyle(line)}>
                    <td>${inventoryCountPhotoHtml(line)}</td>
                    <td>
                        ${inventoryCountMaterialLink(line.materialID)}<br>
                        <small>${escapeHtml(line.materialName)}</small>
                    </td>
                    <td>${escapeHtml(line.uom)}</td>
                    <td>${inventoryCountFormat(system)}</td>
                    <td>
                        <input type="text" inputmode="decimal"
                            id="cnt-counted-${index}"
                            autocomplete="off" value="${countedText}"
                            style="width: 110px;"
                            oninput="updateInventoryCountRow(${index})">
                    </td>
                    <td id="cnt-diff-${index}">
                        ${inventoryCountDifferenceHtml(line)}
                    </td>
                    <td>
                        <select id="cnt-reason-${index}"
                            ${line.counted && roundQty3(line.difference) !== 0 ? "" : "disabled"}>
                            ${inventoryCountReasonOptions(line.reason)}
                        </select>
                    </td>
                    <td>
                        <input type="text" id="cnt-comment-${index}"
                            value="${escapeHtml(line.comment || "")}"
                            placeholder="Optional">
                    </td>
                </tr>`;
        }
        else {

            rows += `
                <tr ${inventoryCountRowStyle(line)}>
                    <td>${inventoryCountPhotoHtml(line)}</td>
                    <td>
                        ${inventoryCountMaterialLink(line.materialID)}<br>
                        <small>${escapeHtml(line.materialName)}</small>
                    </td>
                    <td>${escapeHtml(line.uom)}</td>
                    <td>${inventoryCountFormat(system)}</td>
                    <td>${line.counted ? countedText : "not counted"}</td>
                    <td>${inventoryCountDifferenceHtml(line)}</td>
                    <td>${escapeHtml(line.reason || "")}</td>
                    <td>${escapeHtml(line.comment || "")}</td>
                </tr>`;
        }
    });

    if (state.lines.length === 0) {

        rows = `
            <tr>
                <td colspan="8">
                    This Warehouse has no stocked materials. Add the
                    materials you want to count below.
                </td>
            </tr>`;
    }

    const addBlock = editable ? `
        <div class="multi-line-header" style="margin-top: 20px;">
            <label>Add a material (also with stock 0)</label>
        </div>
        <div style="display: flex; gap: 10px; align-items: flex-start;">
            <div style="flex: 1;">
                ${materialComboboxHtml("cntadd", "Search by ID or name...")}
            </div>
            <button type="button" class="add-line-button"
                onclick="addInventoryCountMaterial()">
                + Add Material
            </button>
        </div>` : "";

    const dateBlock =
        state.status === "Applied" ? `Finished: ${escapeHtml(state.endDate)}` :
        state.status === "Cancelled" ? "Cancelled - stock was not changed" :
        "Not finished yet";

    const actions = editable ? `
        <label style="margin-top: 18px;">Finish date (used when applying)</label>
        <input type="date" id="inventory-count-end"
            value="${inventoryCountToday()}" style="max-width: 200px;">

        <div class="form-actions" style="gap: 10px; flex-wrap: wrap;">
            <button onclick="saveInventoryCountDraft()">Save Draft</button>
            <button class="button-accent"
                onclick="applyInventoryCount()">Apply Count</button>
            <button class="button-muted"
                onclick="cancelInventoryCount()">Cancel Count</button>
        </div>` : "";

    content.innerHTML = `

        <div class="form-container form-container-wide"
            style="max-width: 1200px;">

            <h2>
                ${escapeHtml(state.id)}
                <span class="status-badge ${inventoryCountStatusClass(state.status)}"
                    style="margin-left: 8px;">
                    ${escapeHtml(state.status)}
                </span>
            </h2>

            <p>
                Warehouse ${state.warehouseID} -
                ${escapeHtml(state.warehouseName)}<br>
                Started: ${escapeHtml(state.startDate)} &nbsp;|&nbsp;
                ${dateBlock}
            </p>

            ${state.status === "Open" ? `
            <p>
                Stock movements in this Warehouse are suspended until
                the count is applied or cancelled. Leave "Counted"
                empty for materials you have not counted yet - they
                stay unchanged when the count is applied. The system
                quantity is read when you apply: it cannot change
                meanwhile.
            </p>` : ""}

            ${state.lines.some(l => l.internalSupplier) ? `
            <p>
                <span style="background: #ede9fe; padding: 2px 10px; border-radius: 6px;">
                    Purple rows
                </span>
                are materials of an Internal Supplier (self-made).
            </p>` : ""}

            <label>General comment</label>

            <textarea id="inventory-count-comment" rows="2"
                ${editable ? "" : "disabled"}>${escapeHtml(state.comment)}</textarea>

            <div class="material-table-container" style="margin-top: 16px;">
                <table class="material-table">
                    <thead>
                        <tr>
                            <th>Photo</th>
                            <th>Material</th>
                            <th>UoM</th>
                            <th>System</th>
                            <th>Counted</th>
                            <th>Difference</th>
                            <th>Reason</th>
                            <th>Comment</th>
                        </tr>
                    </thead>
                    <tbody>${rows}</tbody>
                </table>
            </div>

            ${addBlock}

            ${actions}

            <div class="form-actions" style="gap: 10px; flex-wrap: wrap;">
                <button class="button-muted"
                    onclick="exportInventoryCountToExcel()">
                    Export to Excel
                </button>
                <button class="button-muted"
                    onclick="showInventoryCounts()">
                    Back to list
                </button>
            </div>

            <div id="inventory-count-message"></div>

        </div>
    `;

    if (editable) {
        initMaterialCombobox("cntadd", "");
    }
}

// Live difference / reason handling while typing a counted quantity.

function updateInventoryCountRow(index) {

    const line = inventoryCountState.lines[index];

    const input = document.getElementById(`cnt-counted-${index}`);

    const diffCell = document.getElementById(`cnt-diff-${index}`);

    const reasonSelect = document.getElementById(`cnt-reason-${index}`);

    if (!line || !input || !diffCell || !reasonSelect) {
        return;
    }

    const system = roundQty3(line.systemQuantity);

    if (input.value.trim() === "" ||
        !Number.isFinite(Number(inventoryCountParse(input.value)))) {

        diffCell.textContent = "-";

        reasonSelect.value = "";

        reasonSelect.disabled = true;

        return;
    }

    const diff =
        roundQty3(Number(inventoryCountParse(input.value)) - system);

    diffCell.innerHTML =
        inventoryCountDifferenceHtml(
            { counted: true, difference: diff });

    if (diff === 0) {

        reasonSelect.value = "";

        reasonSelect.disabled = true;

        return;
    }

    reasonSelect.disabled = false;

    // Missing / Surplus must match the sign of the difference.
    const current = reasonSelect.value;

    if (current === "" ||
        (current === "Missing material" && diff > 0) ||
        (current === "Surplus material" && diff < 0)) {

        reasonSelect.value =
            diff < 0 ? "Missing material" : "Surplus material";
    }
}

// Reads the sheet. Returns null (after showing the reason) if invalid.

function collectInventoryCountLines() {

    const lines = [];

    for (let index = 0; index < inventoryCountState.lines.length; index++) {

        const source = inventoryCountState.lines[index];

        const input = document.getElementById(`cnt-counted-${index}`);

        const reasonSelect = document.getElementById(`cnt-reason-${index}`);

        const commentInput = document.getElementById(`cnt-comment-${index}`);

        if (!input) {
            continue;
        }

        const text = input.value.trim();

        const line = {
            materialID: source.materialID,
            counted: text !== "",
            comment: commentInput ? commentInput.value.trim() : ""
        };

        if (line.counted) {

            const quantity = Number(inventoryCountParse(text));

            if (!Number.isFinite(quantity) || quantity < 0 ||
                !hasMaxThreeDecimals(quantity)) {

                setInventoryCountMessage(
                    `Counted quantity of ${source.materialID} must be zero or more, with at most 3 decimals.`,
                    true);

                input.focus();

                return null;
            }

            line.countedQuantity = quantity;

            line.reason = reasonSelect ? reasonSelect.value : "";
        }

        lines.push(line);
    }

    return lines;
}

async function postInventoryCountDraft(extraLine) {

    const lines = collectInventoryCountLines();

    if (lines === null) {
        return false;
    }

    if (extraLine) {
        lines.push(extraLine);
    }

    const result =
        await postJson(
            "/api/inventory-counts/save",
            {
                id: inventoryCountState.id,
                comment:
                    document.getElementById("inventory-count-comment")
                        .value.trim(),
                lines
            });

    if (!result.ok) {

        setInventoryCountMessage(result.text, true);

        return false;
    }

    return true;
}

async function saveInventoryCountDraft() {

    if (!(await postInventoryCountDraft())) {
        return;
    }

    await openInventoryCount(
        inventoryCountState.id, "Draft saved. Stock is unchanged.");
}

async function addInventoryCountMaterial() {

    const materialID =
        document.getElementById("cntadd-material-id").value;

    if (!materialID) {

        setInventoryCountMessage("Please select a material to add.", true);

        return;
    }

    if (inventoryCountState.lines.some(l => l.materialID === materialID)) {

        setInventoryCountMessage(
            "That material is already in this count.", true);

        return;
    }

    if (!(await postInventoryCountDraft(
        { materialID, counted: false, comment: "" }))) {
        return;
    }

    await openInventoryCount(
        inventoryCountState.id, `Material ${materialID} added.`);
}

async function applyInventoryCount() {

    const lines = collectInventoryCountLines();

    if (lines === null) {
        return;
    }

    const endDate =
        document.getElementById("inventory-count-end").value;

    if (!endDate) {

        setInventoryCountMessage("Please enter the finish date.", true);

        return;
    }

    const counted = lines.filter(l => l.counted);

    if (counted.length === 0) {

        setInventoryCountMessage(
            "Enter at least one counted quantity before applying.", true);

        return;
    }

    const adjusted =
        counted.filter(l => {
            const source =
                inventoryCountState.lines.find(
                    s => s.materialID === l.materialID);
            return roundQty3(l.countedQuantity -
                (source ? source.systemQuantity : 0)) !== 0;
        }).length;

    const pending = lines.length - counted.length;

    const confirmed =
        await showConfirmDialog(
            `Apply ${inventoryCountState.id}?\n\n` +
            `${adjusted} material(s) will be adjusted to the counted quantity, ` +
            `${counted.length - adjusted} counted without difference, ` +
            `${pending} not counted (unchanged).\n\n` +
            `This cannot be undone. Stock movements in the Warehouse resume afterwards.`);

    if (!confirmed) {
        return;
    }

    if (!(await postInventoryCountDraft())) {
        return;
    }

    const result =
        await postJson(
            "/api/inventory-counts/apply",
            { id: inventoryCountState.id, endDate });

    if (!result.ok) {

        setInventoryCountMessage(result.text, true);

        return;
    }

    await openInventoryCount(
        inventoryCountState.id, JSON.parse(result.text).message);
}

async function cancelInventoryCount() {

    const confirmed =
        await showConfirmDialog(
            `Cancel ${inventoryCountState.id}? The counted quantities are ` +
            `discarded, stock stays unchanged and the Warehouse is unlocked.`);

    if (!confirmed) {
        return;
    }

    const result =
        await postJson(
            "/api/inventory-counts/cancel",
            { id: inventoryCountState.id });

    if (!result.ok) {

        setInventoryCountMessage(result.text, true);

        return;
    }

    await openInventoryCount(
        inventoryCountState.id, JSON.parse(result.text).message);
}

function exportInventoryCountToExcel() {

    const state = inventoryCountState;

    const rows = [[
        "Count", "Warehouse ID", "Warehouse", "Status", "Started",
        "Finished", "General comment", "Material ID", "Material", "UoM",
        "System", "Counted", "Difference", "Reason", "Line comment"
    ]];

    for (const line of state.lines) {

        rows.push([
            state.id, state.warehouseID, state.warehouseName, state.status,
            state.startDate, state.endDate || "", state.comment,
            line.materialID, line.materialName, line.uom,
            roundQty3(line.systemQuantity),
            line.counted ? roundQty3(line.countedQuantity) : "",
            line.counted ? roundQty3(line.difference) : "",
            line.reason || "", line.comment || ""
        ]);
    }

    downloadCSV(`${state.id}_export.csv`, rows);
}


/* ============================================================
   MOVEMENT HISTORY (material) and SALES HISTORY (product)
   Both read the movement ledger: GET /api/movements
============================================================ */

let materialMovementRows = [];

let materialMovementID = "";

function movementQty(value) {

    return value === "" || value === undefined ? "" : value;
}

async function loadMaterialMovements(materialID) {

    const box = document.getElementById("material-movements");

    if (!box) {
        return;
    }

    box.innerHTML = `<p>Loading movements...</p>`;

    try {

        const response =
            await fetch(
                `/api/movements?material=${encodeURIComponent(materialID)}&limit=2000`);

        if (!response.ok) {

            box.innerHTML = `<p>Error: ${escapeHtml(await response.text())}</p>`;

            return;
        }

        const data = await response.json();

        materialMovementRows =
            (data.movements || []).filter(row => row.materialID === materialID);

        materialMovementID = materialID;

        renderMaterialMovements("");
    }
    catch (error) {

        console.error(error);

        box.innerHTML = `<p>Could not load the movement history.</p>`;
    }
}

function renderMaterialMovements(warehouseFilter) {

    const box = document.getElementById("material-movements");

    if (!box) {
        return;
    }

    const warehouses = [];

    for (const row of materialMovementRows) {

        if (row.warehouse && !warehouses.includes(row.warehouse)) {
            warehouses.push(row.warehouse);
        }
    }

    warehouses.sort((a, b) => Number(a) - Number(b));

    const rows =
        materialMovementRows.filter(
            row => !warehouseFilter || row.warehouse === warehouseFilter);

    let body = "";

    for (const row of rows) {

        const detail =
            [row.details, row.comment].filter(Boolean).join(" - ");

        body += `
            <tr>
                <td>${escapeHtml(row.timestamp)}</td>
                <td>${escapeHtml(row.type)}</td>
                <td>${escapeHtml(row.warehouse)}${row.warehouseName ? " - " + escapeHtml(row.warehouseName) : ""}</td>
                <td style="color:#15803d;font-weight:bold;">${row.qtyIn ? "+" + escapeHtml(row.qtyIn) : ""}</td>
                <td style="color:#b91c1c;font-weight:bold;">${row.qtyOut ? "-" + escapeHtml(row.qtyOut) : ""}</td>
                <td>${escapeHtml(row.balance)}</td>
                <td>${escapeHtml(row.reference)}</td>
                <td>${escapeHtml(row.user)}</td>
                <td>${escapeHtml(detail)}</td>
            </tr>`;
    }

    if (rows.length === 0) {

        body = `
            <tr>
                <td colspan="9">No movements recorded for this material.</td>
            </tr>`;
    }

    const options =
        `<option value="">All warehouses</option>` +
        warehouses.map(w => `
            <option value="${escapeHtml(w)}"
                ${w === warehouseFilter ? "selected" : ""}>
                Warehouse ${escapeHtml(w)}
            </option>`).join("");

    box.innerHTML = `

        <div class="procurement-section">

            <h3>Stock Movements</h3>

            <div class="form-actions" style="gap: 10px; flex-wrap: wrap;">

                <select id="material-movement-filter"
                    onchange="renderMaterialMovements(this.value)"
                    style="width: auto;">
                    ${options}
                </select>

                <button class="button-muted"
                    onclick="exportMaterialMovements()">
                    Export to Excel
                </button>

            </div>

            <div class="material-table-container" style="margin-top: 12px;">
                <table class="material-table">
                    <thead>
                        <tr>
                            <th>Date</th>
                            <th>Type</th>
                            <th>Warehouse</th>
                            <th>In</th>
                            <th>Out</th>
                            <th>Balance</th>
                            <th>Reference</th>
                            <th>User</th>
                            <th>Details</th>
                        </tr>
                    </thead>
                    <tbody>${body}</tbody>
                </table>
            </div>

        </div>
    `;
}

function exportMaterialMovements() {

    const filter =
        (document.getElementById("material-movement-filter") || {}).value || "";

    const rows = [[
        "Timestamp", "User", "Type", "Warehouse", "Material ID", "Material",
        "Qty In", "Qty Out", "Balance After", "Reference", "Details", "Comment"
    ]];

    for (const row of materialMovementRows) {

        if (filter && row.warehouse !== filter) {
            continue;
        }

        rows.push([
            row.timestamp, row.user, row.type, row.warehouse,
            row.materialID, row.materialName, row.qtyIn, row.qtyOut,
            row.balance, row.reference, row.details, row.comment
        ]);
    }

    downloadCSV(`${materialMovementID}_movements.csv`, rows);
}

// ------------------------------------------------------------
// Product sales history
// ------------------------------------------------------------

let productSalesRows = [];

async function showProductSalesHistory() {

    const content = document.getElementById("product-content");

    content.innerHTML = `<p>Loading sales history...</p>`;

    try {

        const response =
            await fetch("/api/movements?type=PRODUCT%20SALE&limit=5000");

        if (!response.ok) {

            content.innerHTML = `<p>Error: ${escapeHtml(await response.text())}</p>`;

            return;
        }

        const data = await response.json();

        productSalesRows =
            (data.movements || []).filter(row => row.type === "PRODUCT SALE");

        renderProductSales();
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `<p>Could not connect to the server.</p>`;
    }
}

function renderProductSales() {

    const content = document.getElementById("product-content");

    const selectedProduct =
        (document.getElementById("sales-filter-product") || {}).value || "";

    const from =
        (document.getElementById("sales-filter-from") || {}).value || "";

    const to =
        (document.getElementById("sales-filter-to") || {}).value || "";

    const products = [];

    for (const row of productSalesRows) {

        if (!products.some(p => p.id === row.productID)) {
            products.push({ id: row.productID, name: row.productName });
        }
    }

    products.sort((a, b) => a.id.localeCompare(b.id));

    const rows =
        productSalesRows.filter(row => {

            const day = row.timestamp.substring(0, 10);

            return (!selectedProduct || row.productID === selectedProduct) &&
                (!from || day >= from) &&
                (!to || day <= to);
        });

    let total = 0;

    let body = "";

    for (const row of rows) {

        total += Number(row.qtyOut || 0);

        const source =
            row.reference === "Sell Product" ? "Sell Product" :
            `Projection ${row.reference}`;

        body += `
            <tr>
                <td>${escapeHtml(row.timestamp)}</td>
                <td>${escapeHtml(row.productID)}<br><small>${escapeHtml(row.productName)}</small></td>
                <td>${escapeHtml(source)}</td>
                <td>${escapeHtml(row.warehouse)}${row.warehouseName ? " - " + escapeHtml(row.warehouseName) : ""}</td>
                <td><b>${escapeHtml(row.qtyOut)}</b></td>
                <td>${escapeHtml(row.user)}</td>
                <td>${escapeHtml([row.details, row.comment].filter(Boolean).join(" - "))}</td>
            </tr>`;
    }

    if (rows.length === 0) {

        body = `
            <tr>
                <td colspan="7">No sales recorded.</td>
            </tr>`;
    }

    const options =
        `<option value="">All products</option>` +
        products.map(p => `
            <option value="${escapeHtml(p.id)}"
                ${p.id === selectedProduct ? "selected" : ""}>
                ${escapeHtml(p.id)} - ${escapeHtml(p.name)}
            </option>`).join("");

    content.innerHTML = `

        <div class="form-container form-container-wide"
            style="max-width: 1100px;">

            <h2>Product Sales History</h2>

            <p>
                Every Sell Product and every confirmed Projection.
                Units sold in this view: <b>${total}</b>
            </p>

            <div class="form-actions" style="gap: 10px; flex-wrap: wrap; align-items: center;">

                <select id="sales-filter-product"
                    onchange="renderProductSales()" style="width: auto;">
                    ${options}
                </select>

                <label>From</label>
                <input type="date" id="sales-filter-from" value="${escapeHtml(from)}"
                    onchange="renderProductSales()" style="width: auto;">

                <label>To</label>
                <input type="date" id="sales-filter-to" value="${escapeHtml(to)}"
                    onchange="renderProductSales()" style="width: auto;">

                <button class="button-muted" onclick="exportProductSales()">
                    Export to Excel
                </button>

            </div>

            <div class="material-table-container" style="margin-top: 12px;">
                <table class="material-table">
                    <thead>
                        <tr>
                            <th>Date</th>
                            <th>Product</th>
                            <th>Source</th>
                            <th>Warehouse</th>
                            <th>Qty</th>
                            <th>User</th>
                            <th>Details</th>
                        </tr>
                    </thead>
                    <tbody>${body}</tbody>
                </table>
            </div>

        </div>
    `;
}

function exportProductSales() {

    const selectedProduct =
        (document.getElementById("sales-filter-product") || {}).value || "";

    const from =
        (document.getElementById("sales-filter-from") || {}).value || "";

    const to =
        (document.getElementById("sales-filter-to") || {}).value || "";

    const rows = [[
        "Timestamp", "User", "Product ID", "Product", "Source",
        "Warehouse", "Qty Sold", "Details"
    ]];

    for (const row of productSalesRows) {

        const day = row.timestamp.substring(0, 10);

        if ((selectedProduct && row.productID !== selectedProduct) ||
            (from && day < from) || (to && day > to)) {
            continue;
        }

        rows.push([
            row.timestamp, row.user, row.productID, row.productName,
            row.reference, row.warehouse, row.qtyOut,
            [row.details, row.comment].filter(Boolean).join(" - ")
        ]);
    }

    downloadCSV("product_sales.csv", rows);
}


// ============================================================
// QUOTATIONS (supplier quotations with price tiers by quantity)
// ============================================================
// The newest quotation per Supplier and Material is the current
// one; older ones stay on file as "Replaced" (with their PDF) and
// are shown with the "Replaced (past)" / "All" views. A quotation
// never includes customs / freight - those only appear on invoices.

let quotationList = [];

let quotationDetail = null;

let quotationLineSeq = 0;

function quotationIsViewer() {

    return !!currentUser && currentUser.role === "viewer";
}

function quotationStatusClass(status) {

    if (status === "Valid") return "status-active";

    if (status === "Expired") return "status-partial";

    return "status-inactive";
}

function quotationNumber(value) {

    return String(Math.round(Number(value) * 1000000) / 1000000);
}

function setQuotationMessage(text, isError) {

    const message = document.getElementById("quotation-message");

    if (!message) {
        return;
    }

    message.textContent = text || "";

    message.style.color = isError ? "#b91c1c" : "#15803d";

    message.style.marginTop = "12px";

    message.style.fontWeight = "bold";
}

// Material information in a pop-up over the current view: nothing is
// navigated away from, closing it leaves the page exactly as it was
async function openMaterialFromQuotation(materialID) {

    const existing = document.getElementById("material-popup");

    if (existing) {
        existing.remove();
    }

    const overlay = document.createElement("div");

    overlay.id = "material-popup";

    overlay.className = "app-modal-overlay";

    overlay.style.zIndex = "1600";

    overlay.innerHTML = `
        <div class="app-modal-box"
            style="max-width: 640px; max-height: 85vh; overflow-y: auto;">
            <p>Loading material...</p>
        </div>`;

    const close = () => {
        document.removeEventListener("keydown", onKey);
        overlay.remove();
    };

    const onKey = event => {
        if (event.key === "Escape") {
            close();
        }
    };

    document.addEventListener("keydown", onKey);

    overlay.addEventListener("mousedown", event => {
        if (event.target === overlay) {
            close();
        }
    });

    document.body.appendChild(overlay);

    const box = overlay.firstElementChild;

    try {

        const response = await fetch(
            `/api/materials/${encodeURIComponent(materialID)}`);

        const data = await response.json();

        if (!response.ok) {
            box.innerHTML = `<p>${escapeHtml(data.message || "Material not found.")}</p>
                <div class="app-modal-actions">
                    <button type="button" class="app-modal-button-primary">Close</button>
                </div>`;
            box.querySelector("button").onclick = close;
            return;
        }

        const field = (label, value) => `
            <div class="detail-field">
                <div class="detail-label">${label}</div>
                <div class="detail-value">${escapeHtml(value || "—")}</div>
            </div>`;

        const extra = (data.additionalSuppliers || []).map(link =>
            escapeHtml(link.name || link.supplier || link.supplierName || "") +
            (link.supplierPartNumber ?
                " (P/N " + escapeHtml(link.supplierPartNumber) + ")" : "")
        ).filter(text => text).join("<br>");

        box.innerHTML = `
            <div class="material-detail-header">
                <div class="material-detail-photo">
                    ${data.photo ?
                        `<img src="/${data.photo}" alt="Material photo">` :
                        `<div class="material-detail-photo-placeholder">No photo</div>`}
                </div>
                <div class="material-detail-title">
                    <h2>${escapeHtml(data.name)}</h2>
                    <p class="material-detail-id">${escapeHtml(data.id)}</p>
                    <span class="status-badge ${data.active ? "status-active" : "status-inactive"}">
                        ${data.active ? "Active" : "Inactive"}
                    </span>
                </div>
            </div>
            <div class="material-detail-grid">
                ${field("Description", data.description)}
                ${field("Unit of Measure", data.uom)}
                ${field("Category", data.category)}
                ${field("Type", data.type)}
                ${field("Drawing Number", data.drawingNumber)}
                ${field("Drawing Version", data.drawingVersion)}
                ${field("Manufacturer", data.manufacturer)}
                ${field("Manufacturer Part Number", data.manufacturerPartNumber)}
                ${field("Primary Supplier", data.supplier)}
                ${field("Primary Supplier Part Number", data.supplierPartNumber)}
            </div>
            ${extra ? `<div class="detail-field" style="margin-top:10px;">
                <div class="detail-label">Additional Suppliers</div>
                <div class="detail-value">${extra}</div></div>` : ""}
            <div class="app-modal-actions">
                <button type="button" class="app-modal-button-secondary" id="material-popup-full">
                    Open full page
                </button>
                <button type="button" class="app-modal-button-primary" id="material-popup-close">
                    Close
                </button>
            </div>`;

        box.querySelector("#material-popup-close").onclick = close;

        box.querySelector("#material-popup-full").onclick = () => {
            close();
            openModule("materials");
            showMaterialDetail(materialID);
        };
    }
    catch (error) {

        box.innerHTML = `<p>Could not load the material.</p>
            <div class="app-modal-actions">
                <button type="button" class="app-modal-button-primary">Close</button>
            </div>`;
        box.querySelector("button").onclick = close;
    }
}

// ------------------------------------------------------------
// List
// ------------------------------------------------------------

// Quotations live inside the Pricing module, which shares the
// "purchase-content" area with the other price views - this makes
// sure the quotation area exists inside it.
function ensureQuotationContainer() {

    if (!document.getElementById("quotation-content")) {

        const host = document.getElementById("purchase-content");

        if (host) {
            host.innerHTML = `<div id="quotation-content"></div>`;
        }
    }
}

async function showQuotations(view, supplier, search) {

    ensureQuotationContainer();

    const content = document.getElementById("quotation-content");

    if (!content) {
        return;
    }

    content.innerHTML = `<p>Loading quotations...</p>`;

    try {

        const response = await fetch("/api/quotations?includePast=1");

        if (!response.ok) {

            content.innerHTML =
                `<p>Error: ${escapeHtml(await response.text())}</p>`;

            return;
        }

        const data = await response.json();

        quotationList = data.quotations || [];

        renderQuotationList(
            view || "Current", supplier || "", search || "");
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `<p>Could not connect to the server.</p>`;
    }
}

function quotationFilteredList(view, supplier, search) {

    const text = (search || "").trim().toLowerCase();

    return quotationList.filter(q => {

        if (view === "Current" && q.status === "Replaced") return false;

        if ((view === "Valid" || view === "Expired" ||
            view === "Replaced") && q.status !== view) return false;

        if (supplier && q.supplier !== supplier) return false;

        if (text) {

            const haystack = [
                q.id, q.supplier, q.supplierQuoteNumber, q.comment,
                ...q.lines.map(l => `${l.materialID} ${l.materialName}`)
            ].join(" ").toLowerCase();

            if (!haystack.includes(text)) return false;
        }

        return true;
    });
}

function renderQuotationList(view, supplier, search) {

    const content = document.getElementById("quotation-content");

    const quotations = quotationFilteredList(view, supplier, search);

    const views = [
        ["Current", "Current (Valid + Expired)"],
        ["Valid", "Valid only"],
        ["Expired", "Expired (still used, review)"],
        ["Replaced", "Replaced (past)"],
        ["All", "All"]
    ];

    const viewOptions = views.map(([value, label]) => `
        <option value="${value}" ${value === view ? "selected" : ""}>
            ${label}
        </option>`).join("");

    const suppliers =
        [...new Set(quotationList.map(q => q.supplier))].sort();

    const supplierOptions =
        `<option value="">All suppliers</option>` +
        suppliers.map(name => `
            <option value="${escapeHtml(name)}"
                ${name === supplier ? "selected" : ""}>
                ${escapeHtml(name)}
            </option>`).join("");

    let rows = "";

    for (const q of quotations) {

        const expired = q.status === "Expired";

        rows += `
            <tr>
                <td>
                    <a href="#"
                        onclick="openQuotation('${escapeHtml(q.id)}'); return false;">
                        ${escapeHtml(q.id)}
                    </a>
                </td>
                <td>${escapeHtml(q.supplier)}</td>
                <td>${escapeHtml(q.supplierQuoteNumber || "-")}</td>
                <td>${escapeHtml(q.issueDate)}</td>
                <td style="${expired ? "color:#b91c1c;font-weight:bold;" : ""}">
                    ${escapeHtml(q.validUntil || "-")}
                </td>
                <td>${escapeHtml(q.currency)}</td>
                <td>${q.lines.length}</td>
                <td>
                    <span class="status-badge ${quotationStatusClass(q.status)}">
                        ${escapeHtml(q.status)}
                    </span>
                </td>
                <td>
                    ${q.hasDocument ?
                `<a href="/api/quotations/${encodeURIComponent(q.id)}/document/file"
                            target="_blank">PDF</a>` : "-"}
                </td>
            </tr>`;
    }

    if (quotations.length === 0) {

        rows = `
            <tr>
                <td colspan="9">No quotations found.</td>
            </tr>`;
    }

    content.innerHTML = `

        <div class="form-container form-container-wide"
            style="max-width: 1100px;">

            <h2>Quotations</h2>

            <p>
                The newest quotation of a supplier for a material is
                the current one. Older ones are kept as
                "Replaced" and can be consulted with their PDF. An
                Expired quotation is still used for estimates - check
                with the supplier whether it is still valid.
            </p>

            <div class="form-actions" style="gap: 10px; flex-wrap: wrap;">

                <button onclick="showNewQuotation()">
                    New Quotation
                </button>

                <select id="quotation-view-filter" style="width: auto;"
                    onchange="applyQuotationFilters()">
                    ${viewOptions}
                </select>

                <select id="quotation-supplier-filter" style="width: auto;"
                    onchange="applyQuotationFilters()">
                    ${supplierOptions}
                </select>

                <input type="text" id="quotation-search"
                    placeholder="Search ID, material, quote no..."
                    value="${escapeHtml(search || "")}"
                    style="width: 220px;"
                    oninput="applyQuotationFilters()">

                <button class="button-muted"
                    onclick="exportQuotationsToExcel()">
                    Export to Excel
                </button>

            </div>

            <div class="material-table-container" style="margin-top: 16px;">
                <table class="material-table">
                    <thead>
                        <tr>
                            <th>Quotation</th>
                            <th>Supplier</th>
                            <th>Supplier quote no.</th>
                            <th>Issued</th>
                            <th>Valid until</th>
                            <th>Currency</th>
                            <th>Materials</th>
                            <th>Status</th>
                            <th>PDF</th>
                        </tr>
                    </thead>
                    <tbody>${rows}</tbody>
                </table>
            </div>

        </div>
    `;

    if (search) {

        const box = document.getElementById("quotation-search");

        if (box) {

            box.focus();

            box.setSelectionRange(box.value.length, box.value.length);
        }
    }
}

function applyQuotationFilters() {

    renderQuotationList(
        document.getElementById("quotation-view-filter").value,
        document.getElementById("quotation-supplier-filter").value,
        document.getElementById("quotation-search").value);
}

function exportQuotationsToExcel() {

    const view =
        (document.getElementById("quotation-view-filter") || {}).value ||
        "Current";

    const supplier =
        (document.getElementById("quotation-supplier-filter") || {}).value ||
        "";

    const search =
        (document.getElementById("quotation-search") || {}).value || "";

    const rows = [[
        "Quotation", "Supplier", "Supplier quote no.", "Issued",
        "Valid until", "Status", "Currency", "Exchange rate", "Incoterm",
        "Lead time (weeks)", "Material", "Material name", "MOQ",
        "From quantity", "Unit price", "Unit price (EUR)", "Comment"
    ]];

    for (const q of quotationFilteredList(view, supplier, search)) {

        for (const line of q.lines) {

            for (const tier of line.tiers) {

                rows.push([
                    q.id, q.supplier, q.supplierQuoteNumber, q.issueDate,
                    q.validUntil, q.status, q.currency, q.exchangeRate,
                    q.incoterm, q.leadTimeWeeks, line.materialID,
                    line.materialName, line.moq, tier.minQuantity,
                    tier.unitPrice, tier.unitPriceEUR, q.comment
                ]);
            }
        }
    }

    downloadCSV("quotations.csv", rows);
}

// ------------------------------------------------------------
// Detail
// ------------------------------------------------------------

async function openQuotation(id) {

    ensureQuotationContainer();

    const content = document.getElementById("quotation-content");

    content.innerHTML = `<p>Loading quotation...</p>`;

    try {

        const response =
            await fetch(`/api/quotations/${encodeURIComponent(id)}`);

        if (!response.ok) {

            content.innerHTML =
                `<p>Error: ${escapeHtml(await response.text())}</p>`;

            return;
        }

        quotationDetail = await response.json();

        renderQuotationDetail();
    }
    catch (error) {

        console.error(error);

        content.innerHTML = `<p>Could not connect to the server.</p>`;
    }
}

function renderQuotationDetail() {

    const q = quotationDetail;

    const content = document.getElementById("quotation-content");

    let lineBlocks = "";

    for (const line of q.lines) {

        const tierRows = line.tiers.map(tier => `
            <tr>
                <td>from ${quotationNumber(tier.minQuantity)} ${escapeHtml(line.uom)}
                    ${line.usualQuantity > 0 && line.usualQuantity === tier.minQuantity ?
                `<span class="status-badge status-ordered">usual purchase</span>` : ""}</td>
                <td>${quotationNumber(tier.unitPrice)} ${escapeHtml(q.currency)}</td>
                <td>${q.currency === "EUR" ? "-" :
                quotationNumber(tier.unitPriceEUR) + " EUR"}</td>
            </tr>`).join("");

        lineBlocks += `
            <div style="margin-top: 18px;">
                <strong>
                    <a href="#"
                        onclick="openMaterialFromQuotation('${escapeHtml(line.materialID)}'); return false;">
                        ${escapeHtml(line.materialID)}
                    </a>
                    - ${escapeHtml(line.materialName)}
                </strong>
                ${line.current ? "" :
                `<span class="status-badge status-inactive"
                        style="margin-left: 8px;">Replaced</span>`}
                <div>
                    Minimum order quantity:
                    ${line.moq ? quotationNumber(line.moq) : "-"}
                </div>
                <table class="material-table" style="margin-top: 6px; max-width: 520px;">
                    <thead>
                        <tr>
                            <th>Quantity</th>
                            <th>Unit price</th>
                            <th>Unit price (EUR)</th>
                        </tr>
                    </thead>
                    <tbody>${tierRows}</tbody>
                </table>
            </div>`;
    }

    const viewer = quotationIsViewer();

    content.innerHTML = `

        <div class="form-container form-container-wide"
            style="max-width: 900px;">

            <h2>
                ${escapeHtml(q.id)}
                <span class="status-badge ${quotationStatusClass(q.status)}"
                    style="margin-left: 8px;">
                    ${escapeHtml(q.status)}
                </span>
            </h2>

            ${q.status === "Expired" ? `
                <p style="color:#b91c1c;font-weight:bold;">
                    This quotation passed its valid-until date and is
                    still used for estimates. Ask the supplier for an
                    updated one.
                </p>` : ""}

            ${q.status === "Replaced" ? `
                <p>
                    A newer quotation of this supplier replaced it. It
                    stays on file for price history.
                </p>` : ""}

            <p>
                <strong>Supplier:</strong> ${escapeHtml(q.supplier)}<br>
                <strong>Supplier quote no.:</strong>
                ${escapeHtml(q.supplierQuoteNumber || "-")}<br>
                <strong>Issued:</strong> ${escapeHtml(q.issueDate)}<br>
                <strong>Valid until:</strong> ${escapeHtml(q.validUntil || "-")}<br>
                <strong>Currency:</strong> ${escapeHtml(q.currency)}${q.currency === "USD" ?
            ` (1 USD = ${quotationNumber(q.exchangeRate)} EUR)` : ""}<br>
                <strong>Incoterm:</strong> ${escapeHtml(q.incoterm || "-")}<br>
                <strong>Lead time:</strong> ${q.leadTimeWeeks ? q.leadTimeWeeks + " weeks" : "-"}<br>
                <strong>Transport:</strong> ${q.freightCost > 0 ?
            quotationNumber(q.freightCost) + " " + escapeHtml(q.currency) +
            " (whole quotation)" : "-"}<br>
                <strong>Comment:</strong> ${escapeHtml(q.comment || "-")}
            </p>

            <p>
                <strong>Invoices priced from it:</strong>
                ${(q.invoices || []).length > 0 ?
            q.invoices.map(escapeHtml).join(", ") +
            ` <small>(it cannot be deleted while linked)</small>` : "none"}
            </p>

            <p>
                <strong>PDF:</strong>
                ${q.hasDocument ?
            `<a href="/api/quotations/${encodeURIComponent(q.id)}/document/file"
                        target="_blank">View PDF</a>` : "none on file"}
                ${viewer ? "" : `
                    &nbsp;
                    <label class="button-muted" style="cursor:pointer; padding: 6px 12px;">
                        ${q.hasDocument ? "Replace PDF" : "Attach PDF"}
                        <input type="file" accept="application/pdf" style="display:none;"
                            onchange="uploadQuotationDocument('${escapeHtml(q.id)}', this)">
                    </label>`}
            </p>

            ${lineBlocks}

            <div class="form-actions" style="margin-top: 20px;">

                <button onclick="showEditQuotation('${escapeHtml(q.id)}')">
                    Edit
                </button>

                <button class="button-danger"
                    onclick="deleteQuotation('${escapeHtml(q.id)}')">
                    Delete
                </button>

                <button class="button-muted" onclick="showQuotations()">
                    Back to list
                </button>

            </div>

            <div id="quotation-message"></div>

        </div>
    `;
}

async function uploadQuotationDocument(id, inputEl) {

    const file = inputEl.files[0];

    if (!file) {
        return;
    }

    setQuotationMessage("Uploading...", false);

    try {

        const fileData = await readFileAsBase64(file);

        const response = await fetch(
            `/api/quotations/${encodeURIComponent(id)}/document`,
            {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ fileData: fileData })
            });

        if (!response.ok) {

            setQuotationMessage(await response.text(), true);

            inputEl.value = "";

            return;
        }

        await openQuotation(id);

        setQuotationMessage("PDF saved.", false);
    }
    catch (error) {

        console.error(error);

        setQuotationMessage("Could not upload the PDF.", true);
    }
}

async function deleteQuotation(id) {

    const confirmed = await showConfirmDialog(
        `Delete quotation ${id}? Its PDF stays in the Documents Folder.`);

    if (!confirmed) {
        return;
    }

    try {

        const response = await fetch("/api/quotations/delete", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ id: id })
        });

        if (!response.ok) {

            setQuotationMessage(await response.text(), true);

            return;
        }

        await showQuotations();
    }
    catch (error) {

        console.error(error);

        setQuotationMessage("Could not connect to the server.", true);
    }
}

// ------------------------------------------------------------
// Create / edit form
// ------------------------------------------------------------

async function showNewQuotation() {

    ensureQuotationContainer();

    await renderQuotationForm(null);
}

async function showEditQuotation(id) {

    if (!quotationDetail || quotationDetail.id !== id) {

        await openQuotation(id);
    }

    await renderQuotationForm(quotationDetail);
}

async function renderQuotationForm(existing) {

    const content = document.getElementById("quotation-content");

    // The PDF read for a NEW quotation is attached when it is created
    quotationPendingPdf = null;

    const q = existing || {
        id: "", supplier: "", supplierQuoteNumber: "",
        issueDate: inventoryCountToday(), validUntil: "",
        currency: "EUR", exchangeRate: 1, incoterm: "", comment: "",
        lines: []
    };

    content.innerHTML = `

        <div class="form-container form-container-wide"
            style="max-width: 1000px;">

            <h2>${existing ? "Edit " + escapeHtml(q.id) : "New Quotation"}</h2>

            ${existing ? "" : `
            <div style="background:#eff6ff; border:1px solid #bfdbfe; border-radius:8px; padding:12px; margin-bottom:14px;">
                <strong>Fill in from the supplier's PDF</strong>
                <p style="margin:6px 0;">
                    Reads the quotation (German or English, PDFs with
                    selectable text) and fills in this form. It is only
                    a help: check everything - the yellow marks show
                    what it was not sure about - before creating the
                    quotation. The PDF is attached automatically.
                </p>
                <input type="file" id="quo-reader-file" accept="application/pdf"
                    onchange="readQuotationPdf(this)">
                <div id="quo-reader-status" style="margin-top:8px;"></div>
            </div>`}

            <label>Supplier</label>
            <select id="quo-supplier" onchange="refreshQuotationPartNumbers()"></select>

            <label>Supplier quote number</label>
            <input type="text" id="quo-number"
                value="${escapeHtml(q.supplierQuoteNumber)}"
                placeholder="Number on the supplier's document">

            <label>Issue date</label>
            <input type="date" id="quo-issue" value="${escapeHtml(q.issueDate)}">

            <label>Valid until (optional)</label>
            <input type="date" id="quo-valid" value="${escapeHtml(q.validUntil)}">

            <label>Currency</label>
            <select id="quo-currency" onchange="updateQuotationCurrency()">
                <option value="EUR" ${q.currency === "EUR" ? "selected" : ""}>EUR</option>
                <option value="USD" ${q.currency === "USD" ? "selected" : ""}>USD</option>
            </select>

            <div id="quo-rate-box" style="${q.currency === "USD" ? "" : "display:none;"}">
                <label>Exchange rate (1 USD = ? EUR)</label>
                <input type="number" id="quo-rate" step="any" min="0"
                    value="${q.currency === "USD" ? q.exchangeRate : ""}">
            </div>

            <label>Incoterm (optional)</label>
            <input type="text" id="quo-incoterm"
                value="${escapeHtml(q.incoterm)}" placeholder="e.g. EXW, DAP">

            <label>Lead time (weeks, optional)</label>
            <input type="number" id="quo-lead" min="0" step="1"
                style="width: 160px;"
                value="${q.leadTimeWeeks ? q.leadTimeWeeks : ""}">

            <label>Transport cost (optional)</label>
            <input type="number" id="quo-freight" min="0" step="any"
                style="width: 160px;"
                value="${q.freightCost ? q.freightCost : ""}"
                placeholder="0.00">
            <small>Transport quoted for the whole quotation, in the
                quotation's currency. It is spread over the usual
                quantities of all its materials in the estimated cost.</small>

            <label>Comment (optional)</label>
            <textarea id="quo-comment" rows="2">${escapeHtml(q.comment)}</textarea>

            <h3 style="margin-top: 20px;">Materials and price tiers</h3>

            <p>
                A tier applies from its quantity up to the next tier:
                the quantity bought decides the price. Customs are not
                part of a quotation; transport is one amount for the
                whole quotation (above).
            </p>

            <div id="quo-lines"></div>

            <div class="form-actions">
                <button class="button-muted" onclick="addQuotationLine()">
                    + Add Material
                </button>
            </div>

            <div class="form-actions" style="margin-top: 20px;">

                <button onclick="saveQuotation('${escapeHtml(q.id)}')">
                    ${existing ? "Save Changes" : "Create Quotation"}
                </button>

                <button class="button-muted"
                    onclick="${existing ?
            `openQuotation('${escapeHtml(q.id)}')` : "showQuotations()"}">
                    Cancel
                </button>

            </div>

            <div id="quotation-message"></div>

        </div>
    `;

    await populateSupplierSelect("quo-supplier", q.supplier);

    const supplierSelect = document.getElementById("quo-supplier");

    if (supplierSelect && q.supplier) {
        supplierSelect.value = q.supplier;
    }

    // Suggest the current default exchange rate for a new USD quote
    if (!existing) {

        try {

            const response = await fetch("/api/purchase/config");

            if (response.ok) {

                const data = await response.json();

                const rate = document.getElementById("quo-rate");

                if (rate && data.exchangeRate) {
                    rate.value = data.exchangeRate;
                }
            }
        }
        catch (error) {
            console.error(error);
        }
    }

    quotationLineSeq = 0;

    if (q.lines.length === 0) {

        await addQuotationLine();
    }
    else {

        for (const line of q.lines) {

            await addQuotationLine(line);
        }
    }
}

async function updateQuotationCurrency() {

    const usd =
        document.getElementById("quo-currency").value === "USD";

    document.getElementById("quo-rate-box").style.display =
        usd ? "" : "none";

    // No rate yet (or the 1 of an EUR quotation): the one in Settings
    const rate = document.getElementById("quo-rate");

    if (usd && rate && (rate.value === "" || Number(rate.value) === 1)) {

        rate.value = await loadPurchaseDefaultExchangeRate();
    }
}

// The Supplier of the quotation decides which part number is shown first
function refreshQuotationPartNumbers() {

    for (const box of document.querySelectorAll("#quo-lines .quo-line")) {

        const hidden = document.getElementById(box.id + "-material-id");

        updateMaterialPartNumberHint(box.id, hidden ? hidden.value : "");
    }
}

async function addQuotationLine(line) {

    const container = document.getElementById("quo-lines");

    const n = quotationLineSeq++;

    const prefix = `quo-line-${n}`;

    const wrapper = document.createElement("div");

    wrapper.className = "quo-line";

    wrapper.id = prefix;

    wrapper.style.cssText =
        "border:1px solid #d1d5db; border-radius:8px; padding:12px; margin-bottom:12px;";

    wrapper.innerHTML = `
        <label>Material</label>
        ${materialComboboxHtml(prefix, "Search by ID, name or part number...")}
        <div id="${prefix}-material-id-pn"
            style="margin:6px 2px 0; font-size:14px;"></div>

        <label>Minimum order qty (optional)</label>
        <input type="number" min="0" step="any"
            class="quo-moq" style="width:180px;"
            value="${line && line.moq ? line.moq : ""}">

        <label>Price tiers</label>
        <div class="quo-tiers"></div>

        <details style="margin:8px 0;"
            onpaste="pastePriceTableImage(event, '${prefix}')">
            <summary style="cursor:pointer; color:#1d4ed8;">
                Read the price table of a web shop (RS, Mouser, ...)
            </summary>
            <p style="margin:6px 0;">
                <strong>With a picture:</strong> take a screenshot of
                the quantity / price table (Win+Shift+S), click in the
                box below and press Ctrl+V - or choose an image file.
                The quantities and unit prices are read from it and
                replace the tiers below; the read text stays visible so
                it can be corrected. <strong>With text:</strong> copy
                the table from the page and paste it in the box, then
                press "Read the tiers". Prices are taken in the
                quotation's currency - check it.
            </p>
            <input type="file" class="quo-paste-file" accept="image/*"
                onchange="readPriceTableImageFile(this, '${prefix}')">
            <textarea class="quo-paste" rows="5" style="width:100%; margin-top:6px;"
                placeholder="Paste the screenshot here (Ctrl+V) or text like:&#10;10 - 240   0,086&#10;250 - 990   0,062&#10;1000 - 4990   0,054"></textarea>
            <div class="quo-paste-status" style="margin:6px 0;"></div>
            <div class="form-actions">
                <button class="button-muted" type="button"
                    onclick="applyPastedTiers('${prefix}')">
                    Read the tiers
                </button>
            </div>
        </details>

        <div class="form-actions">
            <button class="button-muted" type="button"
                onclick="addQuotationTier('${prefix}')">
                + Add Tier
            </button>
            <button class="button-muted" type="button"
                onclick="removeQuotationLine('${prefix}')">
                Remove Material
            </button>
        </div>
    `;

    container.appendChild(wrapper);

    await initMaterialCombobox(prefix, line ? line.materialID : "");

    const tiers = line && line.tiers.length > 0 ?
        line.tiers : [{ minQuantity: "", unitPrice: "" }];

    for (const tier of tiers) {

        addQuotationTier(prefix, tier,
            !!line && line.usualQuantity > 0 &&
            Number(tier.minQuantity) === line.usualQuantity);
    }
}

// The "usual" check works like a radio button (one per material)
// but can be unchecked again
function markUsualTier(checkbox) {

    if (!checkbox.checked) {
        return;
    }

    for (const other of checkbox.closest(".quo-tiers")
        .querySelectorAll(".quo-tier-usual")) {

        if (other !== checkbox) {
            other.checked = false;
        }
    }
}

function addQuotationTier(prefix, tier, usual) {

    const box = document.querySelector(`#${prefix} .quo-tiers`);

    if (!box) {
        return;
    }

    const row = document.createElement("div");

    row.className = "quo-tier";

    row.style.cssText =
        "display:flex; gap:10px; align-items:center; margin-bottom:6px;";

    row.innerHTML = `
        <span>from quantity</span>
        <input type="number" min="0" step="any" class="quo-tier-qty"
            style="width:140px;"
            value="${tier ? tier.minQuantity : ""}">
        <span>unit price</span>
        <input type="number" min="0" step="any" class="quo-tier-price"
            style="width:140px;"
            value="${tier ? tier.unitPrice : ""}">
        <button class="button-muted" type="button"
            onclick="this.parentElement.remove()">x</button>
        <label style="display:flex; gap:4px; align-items:center; margin:0; white-space:nowrap;"
            title="The quantity usually bought: its price is the reference price when there is no newer invoice">
            <input type="checkbox" class="quo-tier-usual"
                ${usual ? "checked" : ""}
                onchange="markUsualTier(this)">
            usual purchase
        </label>
    `;

    box.appendChild(row);
}

function removeQuotationLine(prefix) {

    const element = document.getElementById(prefix);

    if (element) {
        element.remove();
    }
}

async function saveQuotation(id) {

    const lines = [];

    for (const element of document.querySelectorAll("#quo-lines .quo-line")) {

        const tiers = [];

        let usualQuantity = 0;

        for (const row of element.querySelectorAll(".quo-tier")) {

            const qty = row.querySelector(".quo-tier-qty").value;

            const price = row.querySelector(".quo-tier-price").value;

            if (qty === "" && price === "") {
                continue;
            }

            if (row.querySelector(".quo-tier-usual").checked &&
                qty !== "") {

                usualQuantity = Number(qty);
            }

            tiers.push({
                minQuantity: qty === "" ? -1 : Number(qty),
                unitPrice: price === "" ? -1 : Number(price)
            });
        }

        const materialID =
            document.getElementById(`${element.id}-material-id`).value;

        lines.push({
            materialID: materialID,
            moq: Number(element.querySelector(".quo-moq").value || 0),
            usualQuantity: usualQuantity,
            tiers: tiers
        });
    }

    const currency = document.getElementById("quo-currency").value;

    const payload = {
        id: id,
        supplier: document.getElementById("quo-supplier").value,
        supplierQuoteNumber: document.getElementById("quo-number").value.trim(),
        issueDate: document.getElementById("quo-issue").value,
        validUntil: document.getElementById("quo-valid").value,
        currency: currency,
        exchangeRate: currency === "USD" ?
            Number(document.getElementById("quo-rate").value || 0) : 1,
        incoterm: document.getElementById("quo-incoterm").value.trim(),
        leadTimeWeeks: Number(document.getElementById("quo-lead").value || 0),
        freightCost: Number(document.getElementById("quo-freight").value || 0),
        comment: document.getElementById("quo-comment").value.trim(),
        lines: lines
    };

    try {

        const response = await fetch(
            id ? "/api/quotations/update" : "/api/quotations/create",
            {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            });

        if (!response.ok) {

            setQuotationMessage(await response.text(), true);

            return;
        }

        const data = await response.json();

        // A PDF that was read to fill the form is attached right away
        let attached = false;

        if (!id && quotationPendingPdf) {

            try {

                const attach = await fetch(
                    `/api/quotations/${encodeURIComponent(data.id)}/document`,
                    {
                        method: "POST",
                        headers: { "Content-Type": "application/json" },
                        body: JSON.stringify({ fileData: quotationPendingPdf })
                    });

                attached = attach.ok;
            }
            catch (error) {
                console.error(error);
            }

            quotationPendingPdf = null;
        }

        await openQuotation(id || data.id);

        setQuotationMessage(
            id ? "Quotation saved." :
                attached ? "Quotation created and its PDF attached." :
                "Quotation created. You can attach its PDF now.",
            false);
    }
    catch (error) {

        console.error(error);

        setQuotationMessage("Could not connect to the server.", true);
    }
}


// ============================================================
// QUOTATION READER (fills the form from the supplier's PDF)
// ============================================================
// The reading itself lives in /lib/quotation-reader.js (pure text
// parsing, tested on real quotations); pdf.js (/lib) extracts the
// text of the PDF in the browser - the file never leaves this
// computer. It only PREPARES the form: the quotation is always
// reviewed and created by the user.

let quotationPendingPdf = null;

let quotationReaderLoading = null;

function loadQuotationReaderLibs() {

    if (window.pdfjsLib && window.QuotationReader) {
        return Promise.resolve();
    }

    if (quotationReaderLoading) {
        return quotationReaderLoading;
    }

    const load = src => new Promise((resolve, reject) => {

        const script = document.createElement("script");

        script.src = src;

        script.onload = resolve;

        script.onerror = () => reject(new Error("Could not load " + src));

        document.head.appendChild(script);
    });

    quotationReaderLoading = Promise.all([
        window.pdfjsLib ? Promise.resolve() : load("/lib/pdf.min.js"),
        window.QuotationReader ? Promise.resolve() : load("/lib/quotation-reader.js")
    ]).then(() => {

        window.pdfjsLib.GlobalWorkerOptions.workerSrc =
            "/lib/pdf.worker.min.js";
    });

    quotationReaderLoading.catch(() => { quotationReaderLoading = null; });

    return quotationReaderLoading;
}

function quotationReaderStatus(html, kind) {

    const box = document.getElementById("quo-reader-status");

    if (!box) {
        return;
    }

    const color = kind === "error" ? "#b91c1c" :
        kind === "warn" ? "#92400e" : "#166534";

    box.innerHTML = `<div style="color:${color};">${html}</div>`;
}

function markQuotationField(id, note) {

    const el = document.getElementById(id);

    if (!el) {
        return;
    }

    el.style.background = "#fef3c7";

    el.title = note || "Check this value";
}

async function readQuotationPdf(input) {

    const file = input.files[0];

    if (!file) {
        return;
    }

    quotationReaderStatus("Reading " + escapeHtml(file.name) + "...", "ok");

    try {

        await loadQuotationReaderLibs();

        const buffer = await file.arrayBuffer();

        const pdf = await window.pdfjsLib.getDocument(
            { data: new Uint8Array(buffer.slice(0)) }).promise;

        let lines = [];

        for (let p = 1; p <= pdf.numPages; p++) {

            const page = await pdf.getPage(p);

            const text = await page.getTextContent();

            lines = lines.concat(window.QuotationReader.buildLines(
                text.items.map(i => ({
                    s: i.str, x: i.transform[4],
                    y: i.transform[5], w: i.width
                }))));
        }

        if (lines.join("").length < 40) {

            quotationReaderStatus(
                "This PDF has no selectable text (a scan or a photo). " +
                "Reading scans is not available yet - fill the form in " +
                "by hand; the PDF will still be attached.", "warn");

            quotationPendingPdf = await readFileAsBase64(file);

            return;
        }

        const suppliersResponse = await fetch("/api/suppliers");

        const suppliersData = await suppliersResponse.json();

        const materials = await loadMaterialCache(true);

        const context = {
            fileName: file.name,
            suppliers: (suppliersData.suppliers || [])
                .map(s => ({ name: s.name })),
            materials: materials.map(m => ({
                id: m.id,
                name: m.name,
                partNumbers: [m.supplierPartNumber].concat(
                    (m.additionalSuppliers || [])
                        .map(link => link.supplierPartNumber))
                    .filter(pn => pn)
            }))
        };

        const draft = window.QuotationReader.parseQuotation(lines, context);

        quotationPendingPdf = await readFileAsBase64(file);

        await applyQuotationDraft(draft, file.name);
    }
    catch (error) {

        console.error(error);

        quotationReaderStatus(
            "Could not read this PDF - fill the form in by hand.", "error");
    }
}

async function applyQuotationDraft(draft, fileName) {

    const set = (id, value) => {

        const el = document.getElementById(id);

        if (el && value !== undefined && value !== "") {
            el.value = value;
        }
    };

    const warnings = draft.warnings.slice();

    // header fields
    const supplierSelect = document.getElementById("quo-supplier");

    if (draft.supplier && supplierSelect) {
        supplierSelect.value = draft.supplier;
    }
    else {
        markQuotationField("quo-supplier", "Choose the supplier");
    }

    set("quo-number", draft.supplierQuoteNumber);

    set("quo-issue", draft.issueDate);

    set("quo-valid", draft.validUntil);

    set("quo-incoterm", draft.incoterm);

    if (draft.leadTimeWeeks > 0) {
        set("quo-lead", draft.leadTimeWeeks);
    }

    set("quo-currency", draft.currency);

    updateQuotationCurrency();

    const comment = document.getElementById("quo-comment");

    if (comment && !comment.value && draft.notes.length > 0) {
        comment.value = "From the PDF: " + draft.notes.join(" | ");
    }

    // what it was not sure about
    for (const warning of draft.warnings) {

        if (/number/i.test(warning)) {
            markQuotationField("quo-number", warning);
        }
        else if (/issue date/i.test(warning)) {
            markQuotationField("quo-issue", warning);
        }
        else if (/validity/i.test(warning)) {
            markQuotationField("quo-valid", warning);
        }
        else if (/lead time/i.test(warning)) {
            markQuotationField("quo-lead", warning);
        }
        else if (/currency/i.test(warning)) {
            markQuotationField("quo-currency", warning);
        }
    }

    // item lines
    const container = document.getElementById("quo-lines");

    container.innerHTML = "";

    quotationLineSeq = 0;

    let tierCount = 0;

    if (draft.lines.length === 0) {

        await addQuotationLine();
    }

    for (const line of draft.lines) {

        const prefix = `quo-line-${quotationLineSeq}`;

        await addQuotationLine({
            materialID: line.materialID,
            moq: line.moq,
            usualQuantity: line.usualQuantity,
            tiers: line.tiers
        });

        tierCount += line.tiers.length;

        const box = document.getElementById(prefix);

        if (box) {

            const info = document.createElement("div");

            info.style.cssText =
                "font-size:13px; margin-bottom:8px; color:#374151;";

            info.innerHTML =
                "From the PDF: " +
                escapeHtml(line.articleNumbers.join(" / ")) +
                (line.description ? " - " + escapeHtml(line.description) : "");

            box.insertBefore(info, box.firstChild);

            if (line.warnings.length > 0) {

                const warn = document.createElement("div");

                warn.style.cssText =
                    "background:#fef3c7; border-radius:6px; padding:6px 8px; " +
                    "margin-bottom:8px; font-size:13px;";

                warn.textContent = line.warnings.join(" ");

                box.insertBefore(warn, info.nextSibling);

                warnings.push(line.warnings[0]);
            }
        }
    }

    const summary =
        `Read ${draft.lines.length} material line(s) and ${tierCount} ` +
        `price tier(s) from ${escapeHtml(fileName)}. ` +
        `<strong>Check everything before creating the quotation.</strong>`;

    const hints = draft.warnings.map(w => `<li>${escapeHtml(w)}</li>`).join("");

    quotationReaderStatus(
        summary + (hints ? `<ul style="margin:6px 0 0 18px;">${hints}</ul>` : ""),
        draft.warnings.length > 0 ? "warn" : "ok");

    // the tiers also say "usual purchase quantity" when the PDF
    // quoted exactly one quantity
}

async function applyPastedTiers(prefix) {

    const box = document.getElementById(prefix);

    if (!box) {
        return;
    }

    try {

        await loadQuotationReaderLibs();
    }
    catch (error) {

        setQuotationMessage("Could not load the reader.", true);

        return;
    }

    const result = window.QuotationReader.analyzeTierTable(
        box.querySelector(".quo-paste").value);

    const status = box.querySelector(".quo-paste-status");

    if (result.tiers.length === 0) {

        status.innerHTML = `<span style="color:#b91c1c;">
            No quantities and prices were found.</span>`;

        return;
    }

    box.querySelector(".quo-tiers").innerHTML = "";

    for (const tier of result.tiers) {
        addQuotationTier(prefix, tier);
    }

    const hints = result.warnings.map(w => `<li>${escapeHtml(w)}</li>`).join("");

    status.innerHTML =
        `<span style="color:${hints ? "#92400e" : "#166534"};">` +
        `${result.tiers.length} price tier(s) read - check them.</span>` +
        (hints ? `<ul style="margin:4px 0 0 18px; color:#92400e;">${hints}</ul>` : "");

    if (hints) {

        for (const input of box.querySelectorAll(".quo-tiers input")) {
            input.style.background = "#fef3c7";
        }
    }
}

// ------------------------------------------------------------
// Price table from a PICTURE (OCR, in the browser, no internet):
// Tesseract.js + English data live in /lib. The picture is enlarged
// first (small screenshots are not read otherwise), then only
// digits and the signs , . - + EUR are accepted.
// ------------------------------------------------------------

let ocrWorker = null;

let ocrLoading = null;

function loadOcrLibs() {

    if (window.Tesseract) {
        return Promise.resolve();
    }

    if (!ocrLoading) {

        ocrLoading = new Promise((resolve, reject) => {

            const script = document.createElement("script");

            script.src = "/lib/tesseract.min.js";

            script.onload = resolve;

            script.onerror = () => reject(new Error("Could not load OCR"));

            document.head.appendChild(script);
        });

        ocrLoading.catch(() => { ocrLoading = null; });
    }

    return ocrLoading;
}

async function getOcrWorker() {

    if (ocrWorker) {
        return ocrWorker;
    }

    await loadOcrLibs();

    const base = window.location.origin + "/lib";

    const worker = await window.Tesseract.createWorker("eng", 1, {
        workerPath: base + "/tesseract-worker.min.js",
        corePath: base,
        langPath: base,
        gzip: true
    });

    await worker.setParameters({
        tessedit_pageseg_mode: "6",
        tessedit_char_whitelist: "0123456789,.€-+ "
    });

    ocrWorker = worker;

    return ocrWorker;
}

async function preparePriceImage(file) {

    const bitmap = await createImageBitmap(file);

    // small pictures are enlarged to about 4x, big ones left alone
    const scale = Math.max(1, Math.min(4, 1600 / bitmap.width));

    const border = 50;

    const canvas = document.createElement("canvas");

    canvas.width = Math.round(bitmap.width * scale) + 2 * border;

    canvas.height = Math.round(bitmap.height * scale) + 2 * border;

    const context = canvas.getContext("2d");

    context.fillStyle = "#ffffff";

    context.fillRect(0, 0, canvas.width, canvas.height);

    context.imageSmoothingQuality = "high";

    context.filter = "grayscale(1)";

    context.drawImage(bitmap, border, border,
        Math.round(bitmap.width * scale), Math.round(bitmap.height * scale));

    return canvas;
}

async function readPriceTableImage(file, prefix) {

    const box = document.getElementById(prefix);

    if (!box || !file) {
        return;
    }

    const status = box.querySelector(".quo-paste-status");

    status.innerHTML = `<span style="color:#166534;">
        Reading the picture (the first time can take a few seconds)...</span>`;

    try {

        const worker = await getOcrWorker();

        const canvas = await preparePriceImage(file);

        const result = await worker.recognize(canvas);

        box.querySelector(".quo-paste").value =
            result.data.text.split(/\r?\n/)
                .map(line => line.trim()).filter(line => line).join("\n");

        await applyPastedTiers(prefix);
    }
    catch (error) {

        console.error(error);

        status.innerHTML = `<span style="color:#b91c1c;">
            Could not read the picture - type the tiers in by hand.</span>`;
    }
}

function readPriceTableImageFile(input, prefix) {

    const file = input.files[0];

    input.value = "";

    readPriceTableImage(file, prefix);
}

function pastePriceTableImage(event, prefix) {

    const items = (event.clipboardData && event.clipboardData.items) || [];

    for (const item of items) {

        if (item.kind === "file" && item.type.startsWith("image/")) {

            event.preventDefault();

            readPriceTableImage(item.getAsFile(), prefix);

            return;
        }
    }
}

// ============================================================
// COST SOURCE: invoice (real) or quotation (estimated)
// ============================================================
// Renders the price of one material as decided by the cost
// estimator (see CostEstimator.h): the unit price, where it comes
// from, the quotation breakdown (tier price + estimated import
// increment) and the warnings. totalQuantity is the quantity the
// price was looked up for.

function renderCostSourceHtml(est, totalQuantity, uom, compact) {

    if (!est || est.source === "None") {

        return "No price yet";
    }

    const small = (text, color) =>
        `<br><small${color ? ` style="color:${color};"` : ""}>${text}</small>`;

    // Price per unit of measure, so the size of the number is clear
    let html = formatMoney(est.unitPriceEUR) +
        ` <small>EUR${uom ? " / " + escapeHtml(uom) : ""}</small>`;

    // Dense version (Procurement tables): the price and where it comes
    // from, nothing else; the quotation warnings shrink to a tooltip
    if (compact) {

        if (est.source === "Invoice") {

            return html + small(`Invoice ${escapeHtml(est.sourceRef)}`);
        }

        const warnings = [];

        if (est.belowFirstTier) {
            warnings.push("quantity is below the first quoted tier");
        }

        if (est.belowMoq) {
            warnings.push("below the minimum order quantity");
        }

        if (est.quotationExpired) {
            warnings.push("quotation expired");
        }

        return html +
            ` <span class="status-badge status-partial">Estimated</span>` +
            small(`Quotation ${escapeHtml(est.sourceRef)}` +
                (warnings.length > 0 ?
                    ` <span style="color:#b91c1c; cursor:help;" ` +
                    `title="${escapeHtml(warnings.join("; "))}">&#9888;</span>` : ""));
    }

    if (est.source === "Invoice") {

        html += small(
            `Invoice ${escapeHtml(est.sourceRef)} &middot; ` +
            `${escapeHtml(est.sourceDate)} (real)`);

        if (est.hasQuoteInfo) {

            html += small(
                `Quotation ${escapeHtml(est.quoteRef)} ` +
                `(${escapeHtml(est.quoteSupplier)}, ${escapeHtml(est.quoteDate)}) ` +
                `offers ${formatMoney(est.quotePriceEUR)} EUR` +
                (totalQuantity ? ` for ${totalQuantity}` : "") +
                (est.quoteBelowFirstTier ?
                    " (below its first tier)" : "") +
                (est.quoteExpired ? " - expired" : "") +
                " - information only", "#1d4ed8");
        }

        return html;
    }

    // Quotation governs: estimated
    html += ` <span class="status-badge status-partial">Estimated</span>`;

    html += small(
        `Quotation ${escapeHtml(est.sourceRef)} &middot; ` +
        `${escapeHtml(est.supplierName)} &middot; ${escapeHtml(est.sourceDate)}`);

    if (est.freightIncrementEUR > 0) {

        html += small(
            `${formatMoney(est.basePriceEUR)} quoted + ` +
            `${formatMoney(est.freightIncrementEUR)} transport quoted` +
            (est.hasImportIncrement ?
                ` + ${formatMoney(est.importIncrementEUR)} customs/freight ` +
                `(estimated from ${escapeHtml(est.incrementRef)})` : ""));
    }
    else if (est.hasImportIncrement) {

        html += small(
            `${formatMoney(est.basePriceEUR)} quoted + ` +
            `${formatMoney(est.importIncrementEUR)} customs/freight ` +
            `(estimated from ${escapeHtml(est.incrementRef)})`);
    }

    if (est.belowFirstTier) {

        html += small(
            "quantity is below the first quoted tier - first tier price shown",
            "#b91c1c");
    }

    if (est.belowMoq) {

        html += small("below the minimum order quantity", "#b91c1c");
    }

    if (est.quotationExpired) {

        html += small("quotation expired - check with the supplier", "#b91c1c");
    }

    if (est.hasLastInvoice) {

        html += small(
            `last invoice ${escapeHtml(est.lastInvoiceRef)} ` +
            `(${escapeHtml(est.lastInvoiceDate)}): ` +
            `${formatMoney(est.lastInvoicePriceEUR)} EUR`, "#6b7280");
    }

    return html;
}

// ============================================================
// QUOTATION LINK ON INVOICE LINES
// ============================================================

// Fills a <select> with the quotations that price this material
// (newest first), each with the price its tier gives for quantity.
async function loadQuoteOptions(selectId, materialID, quantity, selectedID) {

    const select = document.getElementById(selectId);

    if (!select) {
        return;
    }

    select.dataset.loaded = "1";

    try {

        const response = await fetch(
            `/api/materials/${encodeURIComponent(materialID)}/price-info` +
            `?quantity=${encodeURIComponent(quantity)}&lite=1`);

        if (!response.ok) {
            return;
        }

        const info = await response.json();

        let options = `<option value="" data-price-eur="">- no quotation -</option>`;

        let found = false;

        for (const q of (info.quotations || [])) {

            const isSelected = q.id === selectedID;

            if (isSelected) {
                found = true;
            }

            options += `
                <option value="${escapeHtml(q.id)}"
                    data-price-eur="${q.tierPriceEUR}"
                    ${isSelected ? "selected" : ""}>
                    ${escapeHtml(q.id)} - ${escapeHtml(q.supplier)} -
                    ${escapeHtml(q.issueDate)} - ${escapeHtml(q.status)} -
                    ${quotationNumber(q.tierPrice)} ${escapeHtml(q.currency)}
                </option>`;
        }

        if (selectedID && !found) {

            options += `
                <option value="${escapeHtml(selectedID)}"
                    data-price-eur="" selected>
                    ${escapeHtml(selectedID)}
                </option>`;
        }

        select.innerHTML = options;
    }
    catch (error) {

        console.error(error);
    }
}

// Unit cost proposed by the chosen quotation, in the invoice's
// currency (the user can still change it).
function quotePriceForInvoice(select, currencyId, rateId) {

    const option = select.options[select.selectedIndex];

    const priceEUR = option ? Number(option.dataset.priceEur) : 0;

    if (!option || !option.value || !(priceEUR > 0)) {
        return null;
    }

    const currency = document.getElementById(currencyId).value;

    const rate = Number(document.getElementById(rateId).value) || 1;

    const value = currency === "USD" ? priceEUR / rate : priceEUR;

    return Math.round(value * 10000) / 10000;
}

function applyPendingQuote(index) {

    const select = document.getElementById(`purchpending-${index}-quote`);

    const value = quotePriceForInvoice(
        select, "purchpending-currency", "purchpending-rate");

    if (value !== null) {

        document.getElementById(`purchpending-${index}-cost`).value = value;
    }
}

function applyInvoiceQuote(invoiceID, index) {

    const select =
        document.getElementById(`purchinv-${invoiceID}-line-${index}-quote`);

    const value = quotePriceForInvoice(
        select, `purchinv-${invoiceID}-currency`,
        `purchinv-${invoiceID}-rate`);

    if (value !== null) {

        document.getElementById(
            `purchinv-${invoiceID}-line-${index}-cost`).value = value;
    }
}

// "Quote QUO-1: quoted 2.80 -> invoiced 3.00 (+0.20 / +7.1%)" under
// the unit cost of an invoice line priced from a quotation
function purchaseInvoiceQuoteNote(line) {

    if (!line.quotationID) {
        return "";
    }

    if (!(line.quotedUnitPriceEUR > 0)) {

        return `<br><small>Quotation ${escapeHtml(line.quotationID)}</small>`;
    }

    const diff = line.deviationEUR;

    const color = diff > 0.00005 ? "#b91c1c" :
        diff < -0.00005 ? "#15803d" : "#6b7280";

    const sign = diff > 0 ? "+" : "";

    return `<br><small style="color:${color};">
        Quotation ${escapeHtml(line.quotationID)}:
        quoted ${formatMoney(line.quotedUnitPriceEUR)} EUR &rarr;
        invoiced ${formatMoney(line.invoicedUnitCostEUR)} EUR
        (${sign}${formatMoney(diff)} / ${sign}${line.deviationPct.toFixed(1)}%)
    </small>`;
}


// ============================================================
// COLUMN FILTERS (click a table title: sort + value list like Excel)
// ============================================================
// enableColumnFilters(root) turns the titles of every table inside
// root into filter buttons. A click opens a small menu: sort A-Z /
// Z-A, a search box and the list of values of that column to tick.
// Several columns can be combined. Rows hidden here are marked
// data-filtered, so the Excel export leaves them out too.

// A row is hidden when a search box or a column filter hides it
function applyRowVisibility(row) {

    const hidden =
        row.dataset.searchHidden === "1" || row.dataset.colHidden === "1";

    row.style.display = hidden ? "none" : "";

    if (hidden) {
        row.setAttribute("data-filtered", "1");
    }
    else {
        row.removeAttribute("data-filtered");
    }
}

function columnFilterCellText(row, index) {

    const cell = row.cells[index];

    return cell ? cell.textContent.replace(/\s+/g, " ").trim() : "";
}

function enableColumnFilters(root, options) {

    const skip = options && options.skip;

    for (const table of root.querySelectorAll("table")) {

        const headRow = table.querySelector("thead tr:last-child");

        if (!headRow || table.dataset.colFilters === "1") {
            continue;
        }

        table.dataset.colFilters = "1";

        table.classList.add("col-filter-table");

        table._colFilters = new Map();

        Array.from(headRow.children).forEach((th, index) => {

            const label = th.textContent.replace(/\s+/g, " ").trim();

            if (label === "" || (skip && skip.test(label))) {
                return;
            }

            th.classList.add("filterable-th");

            th.insertAdjacentHTML("beforeend",
                `<span class="col-filter-caret">&#9662;</span>`);

            th.addEventListener("click", event => {

                event.stopPropagation();

                openColumnFilterMenu(table, index, th);
            });
        });
    }
}

function closeColumnFilterMenu() {

    const menu = document.getElementById("column-filter-menu");

    if (menu) {
        menu.remove();
    }
}

document.addEventListener("click", event => {

    const menu = document.getElementById("column-filter-menu");

    if (menu && !menu.contains(event.target)) {
        closeColumnFilterMenu();
    }
});

document.addEventListener("keydown", event => {

    if (event.key === "Escape") {
        closeColumnFilterMenu();
    }
});

function columnFilterRows(table) {

    return Array.from(table.querySelectorAll("tbody tr")).filter(
        row => row.cells.length === table.querySelector("thead tr:last-child").children.length);
}

function applyColumnFilters(table) {

    const rows = columnFilterRows(table);

    let shown = 0;

    for (const row of rows) {

        let visible = true;

        for (const [index, allowed] of table._colFilters) {

            if (!allowed.has(columnFilterCellText(row, index))) {
                visible = false;

                break;
            }
        }

        row.dataset.colHidden = visible ? "" : "1";

        applyRowVisibility(row);

        if (visible) {
            shown++;
        }
    }

    // header marks + status line
    const headRow = table.querySelector("thead tr:last-child");

    Array.from(headRow.children).forEach((th, index) => {
        th.classList.toggle("has-filter", table._colFilters.has(index));
    });

    const container = table.closest(".material-table-container") || table.parentElement;

    let status = container.querySelector(".col-filter-status");

    if (table._colFilters.size === 0) {

        if (status) {
            status.remove();
        }

        return;
    }

    if (!status) {

        status = document.createElement("div");

        status.className = "col-filter-status";

        container.insertBefore(status, container.querySelector("table"));
    }

    status.innerHTML = `
        Filtered: ${shown} of ${rows.length} rows
        <a href="#" onclick="event.preventDefault(); clearColumnFilters(this);">
            Clear all filters</a>`;

    status._table = table;
}

function clearColumnFilters(link) {

    const status = link.closest(".col-filter-status");

    const table = status._table;

    table._colFilters.clear();

    applyColumnFilters(table);
}

function sortTableByColumn(table, index, descending) {

    const body = table.tBodies[0];

    const rows = columnFilterRows(table);

    const collator = new Intl.Collator(undefined,
        { numeric: true, sensitivity: "base" });

    rows.sort((a, b) => {

        const result = collator.compare(
            columnFilterCellText(a, index), columnFilterCellText(b, index));

        return descending ? -result : result;
    });

    for (const row of rows) {
        body.appendChild(row);
    }

    const headRow = table.querySelector("thead tr:last-child");

    Array.from(headRow.children).forEach((th, i) => {

        const caret = th.querySelector(".col-filter-caret");

        if (caret) {
            caret.innerHTML = i === index ?
                (descending ? "&#9650;" : "&#9660;") : "&#9662;";
        }
    });
}

function openColumnFilterMenu(table, index, th) {

    closeColumnFilterMenu();

    const rows = columnFilterRows(table);

    const values = [...new Set(rows.map(
        row => columnFilterCellText(row, index)))].sort(
            new Intl.Collator(undefined, { numeric: true, sensitivity: "base" }).compare);

    const current = table._colFilters.get(index);

    const selected = new Set(current || values);

    const menu = document.createElement("div");

    menu.id = "column-filter-menu";

    menu.innerHTML = `
        <button type="button" class="cf-sort" data-dir="asc">Sort A &rarr; Z</button>
        <button type="button" class="cf-sort" data-dir="desc">Sort Z &rarr; A</button>
        <input type="text" class="cf-search" placeholder="Search values...">
        <label class="cf-all"><input type="checkbox" class="cf-all-box"> (Select all)</label>
        <div class="cf-list"></div>
        <button type="button" class="cf-clear">Clear filter</button>
    `;

    document.body.appendChild(menu);

    const rect = th.getBoundingClientRect();

    menu.style.left =
        Math.max(8, Math.min(rect.left, window.innerWidth - 270)) + "px";

    menu.style.top = (rect.bottom + 4) + "px";

    const list = menu.querySelector(".cf-list");

    const search = menu.querySelector(".cf-search");

    const allBox = menu.querySelector(".cf-all-box");

    function shownValues() {

        const text = search.value.trim().toLowerCase();

        return values.filter(
            value => text === "" || value.toLowerCase().includes(text));
    }

    function commit() {

        if (selected.size === values.length) {
            table._colFilters.delete(index);
        }
        else {
            table._colFilters.set(index, new Set(selected));
        }

        applyColumnFilters(table);
    }

    function renderList() {

        const visible = shownValues();

        list.innerHTML = visible.map((value, i) => `
            <label class="cf-item">
                <input type="checkbox" data-i="${i}" ${selected.has(value) ? "checked" : ""}>
                ${value === "" ? "<em>(blank)</em>" : escapeHtml(value)}
            </label>`).join("");

        allBox.checked = visible.length > 0 &&
            visible.every(value => selected.has(value));

        list.querySelectorAll("input").forEach(box => {

            box.addEventListener("change", () => {

                const value = visible[Number(box.dataset.i)];

                if (box.checked) {
                    selected.add(value);
                }
                else {
                    selected.delete(value);
                }

                allBox.checked = visible.every(v => selected.has(v));

                commit();
            });
        });
    }

    allBox.addEventListener("change", () => {

        for (const value of shownValues()) {

            if (allBox.checked) {
                selected.add(value);
            }
            else {
                selected.delete(value);
            }
        }

        renderList();

        commit();
    });

    search.addEventListener("input", renderList);

    menu.querySelectorAll(".cf-sort").forEach(button => {

        button.addEventListener("click", () => {

            sortTableByColumn(table, index, button.dataset.dir === "desc");

            closeColumnFilterMenu();
        });
    });

    menu.querySelector(".cf-clear").addEventListener("click", () => {

        table._colFilters.delete(index);

        applyColumnFilters(table);

        closeColumnFilterMenu();
    });

    renderList();

    search.focus();
}

// ============================================================
// EXPORT TO EXCEL (every view that shows tables)
// ============================================================
// A floating "Export to Excel" button appears whenever the open
// module shows at least one table. It writes a real .xlsx file (no
// library: a minimal OOXML + zip writer) with one sheet per table -
// the tables exactly as they are on screen, including every invoice /
// order card of a list even when collapsed.

const xlsxCrcTable = (() => {

    const table = [];

    for (let n = 0; n < 256; n++) {

        let c = n;

        for (let k = 0; k < 8; k++) {
            c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
        }

        table.push(c >>> 0);
    }

    return table;
})();

function xlsxCrc32(bytes) {

    let crc = 0xFFFFFFFF;

    for (let i = 0; i < bytes.length; i++) {
        crc = xlsxCrcTable[(crc ^ bytes[i]) & 0xFF] ^ (crc >>> 8);
    }

    return (crc ^ 0xFFFFFFFF) >>> 0;
}

// Zip with "stored" (uncompressed) entries: files = [{ name, text }]
function xlsxZip(files) {

    const encoder = new TextEncoder();

    const chunks = [];

    const central = [];

    let offset = 0;

    const u16 = v => [v & 0xFF, (v >>> 8) & 0xFF];

    const u32 = v => [v & 0xFF, (v >>> 8) & 0xFF, (v >>> 16) & 0xFF, (v >>> 24) & 0xFF];

    for (const file of files) {

        const name = encoder.encode(file.name);

        const data = encoder.encode(file.text);

        const crc = xlsxCrc32(data);

        const local = new Uint8Array([
            ...u32(0x04034b50), ...u16(20), ...u16(0x0800), ...u16(0),
            ...u16(0), ...u16(0x21), ...u32(crc), ...u32(data.length),
            ...u32(data.length), ...u16(name.length), ...u16(0)
        ]);

        chunks.push(local, name, data);

        central.push(new Uint8Array([
            ...u32(0x02014b50), ...u16(20), ...u16(20), ...u16(0x0800),
            ...u16(0), ...u16(0), ...u16(0x21), ...u32(crc),
            ...u32(data.length), ...u32(data.length), ...u16(name.length),
            ...u16(0), ...u16(0), ...u16(0), ...u16(0), ...u32(0),
            ...u32(offset)
        ]), name);

        offset += local.length + name.length + data.length;
    }

    let centralSize = 0;

    for (const part of central) {
        centralSize += part.length;
    }

    const end = new Uint8Array([
        ...u32(0x06054b50), ...u16(0), ...u16(0),
        ...u16(files.length), ...u16(files.length),
        ...u32(centralSize), ...u32(offset), ...u16(0)
    ]);

    const all = chunks.concat(central, [end]);

    let total = 0;

    for (const part of all) {
        total += part.length;
    }

    const out = new Uint8Array(total);

    let position = 0;

    for (const part of all) {

        out.set(part, position);

        position += part.length;
    }

    return out;
}

function xlsxEscape(text) {

    return String(text)
        .replace(/[\u0000-\u0008\u000b\u000c\u000e-\u001f]/g, "")
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

function xlsxColumnName(index) {

    let name = "";

    for (let n = index + 1; n > 0; n = Math.floor((n - 1) / 26)) {
        name = String.fromCharCode(65 + ((n - 1) % 26)) + name;
    }

    return name;
}

// sheets = [{ name, title, header: [..], rows: [[..]] }]
// numbers stay numbers, everything else is text
function buildXlsx(sheets) {

    const used = {};

    const sheetNames = sheets.map((sheet, index) => {

        let base = String(sheet.name || ("Sheet " + (index + 1)))
            .replace(/[\\\/\?\*\[\]:]/g, " ").trim().slice(0, 28) || ("Sheet " + (index + 1));

        let name = base;

        for (let n = 2; used[name.toLowerCase()]; n++) {
            name = base.slice(0, 26) + " " + n;
        }

        used[name.toLowerCase()] = true;

        return name;
    });

    const files = [];

    files.push({
        name: "[Content_Types].xml",
        text: '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' +
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">' +
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>' +
            '<Default Extension="xml" ContentType="application/xml"/>' +
            '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>' +
            '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>' +
            sheets.map((s, i) =>
                `<Override PartName="/xl/worksheets/sheet${i + 1}.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>`).join("") +
            '</Types>'
    });

    files.push({
        name: "_rels/.rels",
        text: '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' +
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">' +
            '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>' +
            '</Relationships>'
    });

    files.push({
        name: "xl/workbook.xml",
        text: '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' +
            '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" ' +
            'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"><sheets>' +
            sheetNames.map((name, i) =>
                `<sheet name="${xlsxEscape(name)}" sheetId="${i + 1}" r:id="rId${i + 1}"/>`).join("") +
            '</sheets></workbook>'
    });

    files.push({
        name: "xl/_rels/workbook.xml.rels",
        text: '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' +
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">' +
            sheets.map((s, i) =>
                `<Relationship Id="rId${i + 1}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet${i + 1}.xml"/>`).join("") +
            `<Relationship Id="rId${sheets.length + 1}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>` +
            '</Relationships>'
    });

    // 0 normal, 1 bold, 2 bold on grey (table header)
    files.push({
        name: "xl/styles.xml",
        text: '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' +
            '<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">' +
            '<fonts count="2"><font><sz val="11"/><name val="Calibri"/></font>' +
            '<font><b/><sz val="11"/><name val="Calibri"/></font></fonts>' +
            '<fills count="3"><fill><patternFill patternType="none"/></fill>' +
            '<fill><patternFill patternType="gray125"/></fill>' +
            '<fill><patternFill patternType="solid"><fgColor rgb="FFD9E1F2"/><bgColor indexed="64"/></patternFill></fill></fills>' +
            '<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>' +
            '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>' +
            '<cellXfs count="3">' +
            '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>' +
            '<xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1"/>' +
            '<xf numFmtId="0" fontId="1" fillId="2" borderId="0" xfId="0" applyFont="1" applyFill="1"/>' +
            '</cellXfs><cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles></styleSheet>'
    });

    sheets.forEach((sheet, index) => {

        const allRows = [];

        if (sheet.title) {
            allRows.push({ cells: [sheet.title], style: 1 });
        }

        if (sheet.summary && sheet.summary.length > 0) {

            for (const line of sheet.summary) {
                allRows.push({ cells: line, style: 0 });
            }

            allRows.push({ cells: [], style: 0 });
        }

        const headerRowNumber = allRows.length + 1;

        allRows.push({ cells: sheet.header, style: 2 });

        for (const row of sheet.rows) {
            allRows.push({ cells: row, style: 0 });
        }

        const columnCount = Math.max(...allRows.map(r => r.cells.length), 1);

        const widths = new Array(columnCount).fill(8);

        allRows.forEach((row, rowIndex) => {

            if (sheet.title && rowIndex === 0) {
                return;
            }

            row.cells.forEach((value, c) => {
                widths[c] = Math.max(widths[c], Math.min(String(value).length + 2, 60));
            });
        });

        let xml = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' +
            '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">' +
            `<sheetViews><sheetView workbookViewId="0"><pane ySplit="${headerRowNumber}" topLeftCell="A${headerRowNumber + 1}" activePane="bottomLeft" state="frozen"/></sheetView></sheetViews>` +
            '<cols>' + widths.map((w, c) =>
                `<col min="${c + 1}" max="${c + 1}" width="${w}" customWidth="1"/>`).join("") + '</cols>' +
            '<sheetData>';

        allRows.forEach((row, rowIndex) => {

            xml += `<row r="${rowIndex + 1}">`;

            row.cells.forEach((value, c) => {

                const ref = xlsxColumnName(c) + (rowIndex + 1);

                const text = String(value === null || value === undefined ? "" : value);

                if (text === "") {
                    return;
                }

                const isNumber = row.style === 0 &&
                    /^-?\d{1,15}(\.\d+)?$/.test(text) && !/^-?0\d/.test(text);

                xml += isNumber ?
                    `<c r="${ref}"><v>${text}</v></c>` :
                    `<c r="${ref}" t="inlineStr"${row.style ? ` s="${row.style}"` : ""}><is><t xml:space="preserve">${xlsxEscape(text)}</t></is></c>`;
            });

            xml += '</row>';
        });

        xml += '</sheetData>';

        if (sheet.rows.length > 0) {

            xml += `<autoFilter ref="A${headerRowNumber}:${xlsxColumnName(columnCount - 1)}${allRows.length}"/>`;
        }

        xml += '</worksheet>';

        files.push({ name: `xl/worksheets/sheet${index + 1}.xml`, text: xml });
    });

    return xlsxZip(files);
}


// --- reading the tables of the open module ----------------------

function exportCellText(cell) {

    const field = cell.querySelector("input:not([type=hidden]):not([type=checkbox]), textarea, select");

    if (field) {

        if (field.tagName === "SELECT") {

            const option = field.options[field.selectedIndex];

            return option ? option.textContent.trim() : "";
        }

        return field.value;
    }

    const box = cell.querySelector("input[type=checkbox]");

    if (box && cell.textContent.trim() === "") {
        return box.checked ? "Yes" : "No";
    }

    return cell.textContent.replace(/\s+/g, " ").trim();
}

function exportTitleOf(table) {

    const card = table.closest(".procurement-card, .procurement-section");

    if (card) {

        const summary = card.querySelector(".procurement-summary");

        if (summary) {

            return {
                name: (summary.querySelector(".procurement-summary-id") || summary)
                    .textContent.replace(/\s+/g, " ").trim(),
                title: summary.textContent.replace(/\s+/g, " ").trim()
            };
        }

        const heading = card.querySelector("h2, h3");

        if (heading) {

            const text = heading.textContent.replace(/\s+/g, " ").trim();

            return { name: text, title: text };
        }
    }

    // nearest heading before the table
    let node = table;

    while (node && node.id !== "module-content") {

        let sibling = node.previousElementSibling;

        while (sibling) {

            const heading = /^H[1-4]$/.test(sibling.tagName) ? sibling :
                sibling.querySelector && sibling.querySelector("h2, h3, h4");

            if (heading) {

                const text = heading.textContent.replace(/\s+/g, " ").trim();

                return { name: text, title: text };
            }

            sibling = sibling.previousElementSibling;
        }

        node = node.parentElement;
    }

    return { name: "", title: "" };
}

function collectExportSheets() {

    const root = document.getElementById("module-content");

    const sheets = [];

    if (!root) {
        return sheets;
    }

    for (const table of root.querySelectorAll("table")) {

        const headerCells = Array.from(
            table.querySelectorAll("thead tr:last-child th"));

        let bodyRows = Array.from(table.querySelectorAll("tbody tr"));

        if (bodyRows.length === 0) {
            bodyRows = Array.from(table.querySelectorAll("tr")).slice(headerCells.length ? 1 : 0);
        }

        // Price columns (cells marked data-export-value) are exported as
        // two columns: the number alone, and the comment next to it
        // (a column marked with data-export-unit: number + unit instead)
        const splitColumns = new Set();

        const unitColumns = new Set();

        for (const tr of bodyRows) {

            Array.from(tr.children).forEach((cell, i) => {

                if (cell.dataset && cell.dataset.exportValue !== undefined) {

                    splitColumns.add(i);

                    if (cell.dataset.exportUnit !== undefined) {
                        unitColumns.add(i);
                    }
                }
            });
        }

        const rows = bodyRows
            .filter(tr => !tr.hasAttribute("data-filtered"))
            .map(tr => {

                const cells = [];

                Array.from(tr.children).forEach((cell, i) => {

                    if (!splitColumns.has(i)) {

                        cells.push(exportCellText(cell));

                        return;
                    }

                    const value = cell.dataset.exportValue || "";

                    if (unitColumns.has(i)) {

                        cells.push(value, cell.dataset.exportUnit || "");

                        return;
                    }

                    // line breaks become spaces so the words do not stick together
                    const copy = cell.cloneNode(true);

                    copy.querySelectorAll("br").forEach(br => br.replaceWith(" "));

                    const note = exportCellText(copy)
                        .replace(/^\s*-?[\d.,]+\s*EUR(\s*\/\s*\S+)?\s*/, "")
                        .replace(/\s*price tiers\s*$/, "")
                        .trim();

                    cells.push(value, note);
                });

                return cells;
            })
            .filter(cells => cells.some(text => text !== ""));

        if (rows.length === 0) {
            continue;
        }

        let header = [];

        headerCells.forEach((th, i) => {

            const label = th.getAttribute("title") && th.textContent.trim().length < 12 ?
                th.getAttribute("title") : th.textContent.replace(/\s+/g, " ").trim();

            header.push(label.replace(/[\u25B2\u25BC\u25BE]/g, "").trim());

            if (splitColumns.has(i)) {
                header.push(unitColumns.has(i) ? "UoM" : "Price comment");
            }
        });

        if (header.length === 0) {

            const first = table.querySelector("tr");

            header = first ? Array.from(first.children).map(exportCellText) : [];

            rows.shift();
        }

        const title = exportTitleOf(table);

        // Totals shown above the table (Product Cost) go to the top of the sheet
        const section = table.closest(".procurement-section");

        const holder = section && section.querySelector("[data-summary]");

        let summary = [];

        if (holder) {

            try {
                summary = JSON.parse(holder.dataset.summary);
            }
            catch (error) {
                summary = [];
            }
        }

        sheets.push({
            name: title.name,
            title: title.title,
            summary: summary,
            header: header,
            rows: rows
        });
    }

    return sheets;
}

function exportCurrentViewToExcel() {

    const sheets = collectExportSheets();

    if (sheets.length === 0) {

        alert("There is no table to export in this view.");

        return;
    }

    const heading = document.querySelector("#module-content h1, #module-content h2");

    const base = (heading ? heading.textContent : "export")
        .replace(/\s+/g, " ").trim().replace(/[^A-Za-z0-9 _-]/g, "").trim() || "export";

    const bytes = buildXlsx(sheets);

    const blob = new Blob([bytes], {
        type: "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"
    });

    const url = URL.createObjectURL(blob);

    const link = document.createElement("a");

    link.href = url;

    link.download = base.replace(/ /g, "_") + "_" +
        new Date().toISOString().slice(0, 10) + ".xlsx";

    document.body.appendChild(link);

    link.click();

    document.body.removeChild(link);

    setTimeout(() => URL.revokeObjectURL(url), 2000);
}

// --- the floating button ----------------------------------------

(function setupExportButton() {

    let timer = null;

    function refresh() {

        const button = document.getElementById("export-excel-button");

        const view = document.getElementById("module-view");

        if (!button || !view) {
            return;
        }

        const visible = !view.classList.contains("hidden") &&
            collectExportSheets().length > 0;

        button.style.display = visible ? "block" : "none";
    }

    function start() {

        if (document.getElementById("export-excel-button")) {
            return;
        }

        const button = document.createElement("button");

        button.id = "export-excel-button";

        button.type = "button";

        button.textContent = "Export to Excel";

        button.title = "Download the tables of this view as an Excel file";

        button.style.display = "none";

        button.onclick = exportCurrentViewToExcel;

        document.body.appendChild(button);

        const content = document.getElementById("module-content");

        const view = document.getElementById("module-view");

        const observer = new MutationObserver(() => {

            clearTimeout(timer);

            timer = setTimeout(refresh, 250);
        });

        if (content) {
            observer.observe(content, { childList: true, subtree: true });
        }

        if (view) {
            observer.observe(view, { attributes: true, attributeFilter: ["class"] });
        }
    }

    if (document.readyState === "loading") {
        document.addEventListener("DOMContentLoaded", start);
    }
    else {
        start();
    }
})();
